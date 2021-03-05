#include "export.h"

#include <thread>
#include <memory>

#include <jni.h>

#include "forwarder.h"

namespace sun {
    class JniHelper {
    public:
        static bool ClearException(JNIEnv *env);

        static JNIEnv *GetEnv(JavaVM *vm);
    };

    bool JniHelper::ClearException(JNIEnv *env) {
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe(); // dump exception info to stderr
            env->ExceptionClear();
            return true;
        }
        return false;
    }

    JNIEnv *JniHelper::GetEnv(JavaVM *vm) {
        JNIEnv *env{};
        if (vm->GetEnv((void **) &env, JNIVRESION) < 0) {
        }
        return env;
    }

    struct NativeConfig {
        int forwarder_port{FORWARDER_DEFAULTPORT};

        void load(JNIEnv *env, jobject obj) {
            if (!obj) { // null
                return;
            }
            auto cls = env->GetObjectClass(obj);
            auto field_id = env->GetFieldID(cls, "forwarderPort"/*java style*/, JSIG_INT);
            if (JniHelper::ClearException(env)) { // reflect failure
                return;
            }
            forwarder_port = env->GetIntField(obj, field_id);
        }
    };

    namespace jni {
        static int stop(JNIFUNCTIONARGS(,));
    }

    static struct JniContext {
        JavaVM *vm;
        std::thread service;
        std::unique_ptr<sun::Forwarder> forwarder;
        NativeConfig cfg;
        bool initialized{false};

        ~JniContext() {
            jni::stop(0, 0);
        }
    } ctx;

    namespace jni {
        class LocalRef {
        public:
            LocalRef(JNIEnv *env, void *obj) : env_(env), obj_(obj) {}

            ~LocalRef() {
                // delete by ourself instead of jvm (by lazy)
                env_->DeleteLocalRef(reinterpret_cast<jclass>(obj_));
            }

            explicit operator jclass() const {
                return reinterpret_cast<jclass>(obj_);
            }

            explicit operator bool() const {
                return obj_;
            }

        private:
            JNIEnv *env_;
            void *obj_;
        };

        static int start(JNIFUNCTIONARGS(env,), jobject cfg) {
            JNIFUNCTION_TRACEENTRY;
            if (ctx.initialized) {
                return EXPORT_SUCCESS;
            }
            ctx.cfg.load(env, cfg);
            ctx.forwarder.reset(new sun::Forwarder(true));
            ctx.forwarder->setPort(ctx.cfg.forwarder_port);
            ctx.forwarder->initialize();
            if (ctx.forwarder->error_count) {
                return EXPORT_FAILURE;
            }
            ctx.service = std::thread([] {
                ctx.forwarder->loop();
            });
            ctx.initialized = true;
            return EXPORT_SUCCESS;
        }

        static int stop(JNIFUNCTIONARGS(,)) {
            JNIFUNCTION_TRACEENTRY;
            if (ctx.initialized && ctx.service.joinable()) {
                ctx.forwarder->pollInstance().quit();
                ctx.service.join();
            }
            return EXPORT_SUCCESS;
        }
    }
}

static JNINativeMethod jni_methods[] = {
        {(char *) "start", (char *) JSIGLIST(PARAMLEFT, OBJECT, PARAMRIGHT, INT), (void *) &sun::jni::start},
        {(char *) "stop",  (char *) JSIGLIST(PARAMLEFT, PARAMRIGHT, INT),         (void *) &sun::jni::stop}
};

// helper macros
#ifndef JNIEXPORT
#define JNIEXPORT
#endif
#ifndef JNICALL
#define JNICALL
#endif

JNIEXPORT jint JNICALL JNI_OnLoad(JNIHOOKPOINTARGS(vm,)) {
    if (sun::util::FileHelper::Exist(FORWARDER_ENABLELOGFILE)) {
        sun::util::RedirectOutput(FORWARDER_LOGFILE);
    }
    sun::ctx.vm = vm;
    do {
        auto env = sun::JniHelper::GetEnv(vm);
        if (!env) {
            FUNCLOG("fail to get java ENV");
            break;
        }
        sun::jni::LocalRef clz(env, env->FindClass(JNICLASS));
        if (!bool(clz)) {
            FUNCLOG("java class [%s] not found", JNICLASS);
            break;
        }
        if (env->RegisterNatives(jclass(clz), jni_methods, DIMENSIONOF(jni_methods)) < 0) {
            FUNCLOG("fail to register native methods");
            break;
        }
        return JNIVRESION;
    } WHEREFALSE;
    return JNI_ERR;
}

JNIEXPORT void JNICALL JNI_OnUnload(JNIHOOKPOINTARGS(vm,)) {
    sun::jni::stop(0, 0);
}