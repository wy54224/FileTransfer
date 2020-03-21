#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push)
#pragma pack(1)

#define true 1
#define false 0

// 基本节点类型
typedef struct Node{
    unsigned char name;
    unsigned int freq;
}Node;

// haffman树的基本类型
typedef struct HaffmanNode{
    Node *node;
    char *code;
    struct HaffmanNode *left, *right;
}HaffmanNode;

// 小根堆
typedef struct MinHeap{
    int size;
    HaffmanNode* arr[256];
}MinHeap;

// 压缩文件头
typedef struct MaffmanHead{
    char flag[3]; //标识符
    int nodenum; //字符种类数
    unsigned long long bitnum; //压缩位数
}MaffmanHead;

// 函数声明
void insert(MinHeap *heap, HaffmanNode *node);
HaffmanNode* top(MinHeap *heap);
void pop(MinHeap *heap);
void destroy_all();
void buildHaffmanCode(HaffmanNode *root, char *str, const int height);
void buildHaffmanTree();
void buildHaffmanTreeFromFile(char *filename_in);
void encode(char *filename_in, char *filename_out);
void decode(char *filename_in, char *filename_out);

// 公共变量
// HaffmanTree的数组
HaffmanNode* HTarr = NULL;
int arrlen = 0;
// 记录字符频次的数组
Node *nodefreq = NULL;
int nodenum = 0;


// 堆插入节点
void insert(MinHeap *heap, HaffmanNode *node){
    if(!heap)
        return;
    if(!node)
        return;
    HaffmanNode *tmp;
    int tail = heap->size;
    heap->arr[heap->size++] = node;
    // 逐个和父节点进行比较
    while(tail && heap->arr[tail]->node->freq < heap->arr[tail >> 1]->node->freq){
        // 交换节点值
        tmp = heap->arr[tail];
        heap->arr[tail] = heap->arr[tail >> 1];
        heap->arr[tail >> 1] = tmp;
        //迭代往上比较，直到根节点
        tail >>= 1;
    }
}

// 获取堆的根节点
HaffmanNode* top(MinHeap *heap){
    if(!heap->size)
        return NULL;
    return heap->arr[0];
}

// 弹出堆的根节点
void pop(MinHeap *heap){
    if(!heap->size)
        return;
    int head = 0;
    HaffmanNode *l, *r, *tmp;
    heap->arr[0] = heap->arr[--heap->size];
    while(1){
        l = (head << 1) + 1 < heap->size ? heap->arr[(head << 1) + 1] : NULL;
        r = (head << 1) + 2 < heap->size ? heap->arr[(head << 1) + 2] : NULL;
        if(l){
            if(r && r->node->freq < l->node->freq && r->node->freq < heap->arr[head]->node->freq){
                tmp = heap->arr[head];
                heap->arr[head] = r;
                head = (head << 1) + 2;
                heap->arr[head] = tmp;
            }else if(l->node->freq < heap->arr[head]->node->freq){
                tmp = heap->arr[head];
                heap->arr[head] = l;
                head = (head << 1) + 1;
                heap->arr[head] = tmp;
            }else
                break;
        }else{
            if(r && r->node->freq < heap->arr[head]->node->freq){
                tmp = heap->arr[head];
                heap->arr[head] = r;
                head = (head << 1) + 2;
                heap->arr[head] = tmp;
            }else
                break;
        }
    }
}

// 析构函数
void destroy_all(){
    int i;
    free(nodefreq);
    for(i = 0; i < nodenum; ++i)
        free(HTarr[i].code);
    free(HTarr);
}

// 递归遍历哈夫曼树，构造哈夫曼编码
void buildHaffmanCode(HaffmanNode *root, char *str, const int height){
    // 如果不是叶子节点
    if(root->left && root->right){
        str[height] = '0';
        buildHaffmanCode(root->left, str, height + 1);
        str[height] = '1';
        buildHaffmanCode(root->right, str, height + 1);
        str[height] = '\0';
    }else{
        root->code = (char*)calloc(sizeof(char), height + 1);
        strcpy(root->code, str);
    }
}

// 构造Haffman树的数据结构
void buildHaffmanTree(){
    // 小根堆：用于haffman树的构建，直接取出最小的节点
    MinHeap heap;
    int i;
    HaffmanNode *l, *r;
    char *str;
    // 申请Haffman树的内存
    HTarr = (HaffmanNode*)calloc(sizeof(HaffmanNode), arrlen);
    for(i = 0; i < nodenum; ++i){
        HTarr[i].node = &nodefreq[i];
        HTarr[i].code = NULL;
        HTarr[i].left = NULL;
        HTarr[i].right = NULL;
        insert(&heap, &HTarr[i]);
    }
    for(i = nodenum; i < arrlen; ++i){
        l = top(&heap);
        pop(&heap);
        r = top(&heap);
        pop(&heap);
        nodefreq[i].freq = l->node->freq + r->node->freq;
        HTarr[i].node = &nodefreq[i];
        HTarr[i].code = NULL;
        HTarr[i].left = l;
        HTarr[i].right = r;
        insert(&heap, &HTarr[i]);
    }
    // dfs构造Haffman Code（数的高度不会超过叶子节点数）
    str = (char*)calloc(sizeof(char), nodenum);
    memset(str, 0, sizeof(str));
    buildHaffmanCode(top(&heap), str, 0);
    free(str);
}

// 读取文件构建哈夫曼树
void buildHaffmanTreeFromFile(char *filename_in){
    FILE *fin;
    int freq[256], i;
    unsigned char c;
    nodenum = 0;
    if((fin = fopen(filename_in, "rb")) == NULL){
        printf("文件名错误\n");
        return ;
    }
    memset(freq, 0, sizeof(freq));
    // 获取每个字符的频次以及字符的数量
    while(true){
        c = fgetc(fin);
        if(feof(fin))
            break;
        // 统计有多少个字符
        if(!freq[c])
            ++nodenum;
        ++freq[c];
    }
    fclose(fin);

    // 此处申请的大小预留了haffman树非叶子节点的值（完全二叉树的节点数为叶子节点数乘2减1）
    arrlen = (nodenum << 1) - 1;
    nodefreq = (Node*)calloc(sizeof(Node), arrlen);
    nodenum = 0;
    // 构造节点类型数组
    for(i = 0; i < 256; ++i)
        if(freq[i]){
            nodefreq[nodenum].name = i;
            nodefreq[nodenum++].freq = freq[i];
        }
    // 构建哈夫曼树
    buildHaffmanTree();
}

void encode(char *filename_in, char *filename_out){
    FILE *fin, *fout;
    unsigned char c, value;
    char *code;
    // 字符到nodefreq和HTarr对应下标转换的数组
    int char2index[256], i, bitcount;
    MaffmanHead head = {'H', 'M', 'T', nodenum, 0};
    // 初始化
    memset(char2index, 0, sizeof(char2index));
    for(i = 0; i < nodenum; ++i)
        char2index[HTarr[i].node->name] = i;
    if((fin = fopen(filename_in, "rb")) == NULL){
        printf("文件名错误\n");
        return ;
    }
    if((fout = fopen(filename_out, "wb")) == NULL){
        printf("输出文件错误\n");
        return ;
    }
    value = 0;
    head.bitnum = 0;
    bitcount = 0;
    // 先将文件头输出，最后再做修改
    fwrite(&head, sizeof(MaffmanHead), 1, fout);
    fwrite(nodefreq, sizeof(Node), nodenum, fout);

    while(true){
        c = fgetc(fin);
        if(feof(fin))
            break;
        // 编码
        code = HTarr[char2index[c]].code;
        for(i = 0; code[i]; ++i){
            // 第bitcount位置1
            if(code[i] == '1')
                value |= 1 << bitcount;
            ++bitcount;
            ++head.bitnum;
            if(bitcount >= 8){
                bitcount = 0;
                fwrite(&value, sizeof(unsigned char), 1, fout);
                value = 0;
            }
        }
    }
    if(bitcount)
        fwrite(&value, sizeof(unsigned char), 1, fout);
    fseek(fout, 0, SEEK_SET);
    fwrite(&head, sizeof(MaffmanHead), 1, fout);
    fclose(fin);
    fclose(fout);
}

void decode(char *filename_in, char *filename_out){
    FILE *fin, *fout;
    unsigned char c;
    MaffmanHead head;
    HaffmanNode *root;
    int i;
    unsigned long long bitcount;
    if((fin = fopen(filename_in, "rb")) == NULL){
        printf("文件名错误\n");
        return ;
    }
    if((fout = fopen(filename_out, "wb")) == NULL){
        printf("输出文件错误\n");
        return ;
    }

    // 读取压缩文件头
    fread(&head, sizeof(MaffmanHead), 1, fin);
    if(head.flag[0] != 'H' || head.flag[1] != 'M' || head.flag[2] != 'T'){
        printf("压缩文件内容错误\n");
        fclose(fin);
        fclose(fout);
        return;
    }
    nodenum = head.nodenum;
    arrlen = (nodenum << 1) - 1;
    nodefreq = (Node*)calloc(sizeof(Node), arrlen);
    fread(nodefreq, sizeof(Node), nodenum, fin);
    // 构建哈夫曼树
    buildHaffmanTree();

    // 遍历哈夫曼树解码：0->left，1->right
    root = &HTarr[arrlen - 1];
    bitcount = 0;
    while(true){
        c = fgetc(fin);
        if(feof(fin))
            break;
        for(i = 0; i < 8; ++i){
            // 当前位是1，向右，否则向左
            if(c & (1 << i))
                root = root->right;
            else
                root = root->left;
            if(!root->left || !root->right){
                fputc(root->node->name, fout);
                root = &HTarr[arrlen - 1];
            }
            ++bitcount;
            if(bitcount >= head.bitnum)
                break;
        }
    }
    if(bitcount < head.bitnum)
        printf("解压文件发生错误\n");

    fclose(fin);
    fclose(fout);
}

#pragma pack(pop)