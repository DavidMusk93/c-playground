all: sender receiver

OBJ_DIR := obj

COMMON_SRC := common.c
COMMON_OBJ := $(addprefix $(OBJ_DIR)/,$(COMMON_SRC:%.c=%.o))
SENDER_SRC := sender.c
SENDER_OBJ := $(addprefix $(OBJ_DIR)/,$(SENDER_SRC:%.c=%.o))
RECEIVER_SRC := receiver.c
RECEIVER_OBJ := $(addprefix $(OBJ_DIR)/,$(RECEIVER_SRC:%.c=%.o))

OBJ := $(COMMON_OBJ) $(SENDER_OBJ) $(RECEIVER_OBJ)

.PHONY: clean
clean:
	@rm -f a.out sender receiver
	@rm -rf $(OBJ_DIR)

$(OBJ): obj/%.o: %.c
	@[ -d $(@D) ] || mkdir -p $(@D)
	gcc -c $< -o $@

sender: $(COMMON_OBJ) $(SENDER_OBJ)
	gcc $^ -o $@

receiver: $(COMMON_OBJ) $(RECEIVER_OBJ)
	gcc $^ -o $@

.PHONY: sync

sync:
	bash helper.sh $@

