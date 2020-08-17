//
// Created by Steve on 8/8/2020.
//

#ifndef C4FUN_BINARY_TREE_H
#define C4FUN_BINARY_TREE_H

struct TreeNode{
    int val;
    TreeNode*left,*right;
    TreeNode(int x,TreeNode*left,TreeNode*right):val(x),left(left),right(right){}
    TreeNode(int x):TreeNode(x,nullptr,nullptr){}
    TreeNode():TreeNode(0){}
};

#endif //C4FUN_BINARY_TREE_H
