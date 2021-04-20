#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <cairo.h>
#include <time.h>

#define TYPE_TML 362
#define TYPE_BLK 198
#define TYPE_NET 754

struct hardblocks{
    int num;
    int cor_x;
    int cor_y;
    int height;
    int width;
    bool rotate;

    int pins_cnt;
    struct list_node *pin_list;

    struct hardblocks *left_child;
    struct hardblocks *right_child;

    struct hardblocks *next;
    struct hardblocks *prev;
};
typedef struct hardblocks hardblock;

struct nets{
    int num;

    int pins_cnt;
    struct list_node *pin_list;

    int hpwl;

    struct nets *next;
    struct nets *prev;
};
typedef struct nets net;

struct tmls{
    int num;
    int pins_cnt;
    int cor_x;
    int cor_y;

    struct tmls *next;
    struct tmls *prev;
};
typedef struct tmls tml;

union list_ptr{
    hardblock *b_ptr;
    tml *t_ptr;
    net *n_ptr;
};
typedef union list_ptr list_ptr;

struct list_node{
    int type;
    int num;
    list_ptr ptr;
    struct list_node *next;
};
typedef struct list_node list_node;

struct sort_node{
    int num;
    struct sort_node *next;
};
typedef struct sort_node sort_node;

FILE *R_file(char *file, char *mode);
void Pins_Num_Check(int BLKS_NUM,int TMLS_NUM, int NETS_NUM, int PINS_NUM, hardblock **blk_arr, tml **tml_arr, net **net_arr);
void W_file(FILE *f, int WIRELENGTH, int BLKS_NUM, hardblock **blks_arr);
void Draw_PNG(int BLKS_NUM, hardblock **blks_arr, int W, int H);
void Sorted_List_Check(sort_node *list, int BLKS_NUM, hardblock **blks_arr);
void Traverse_Bstree_Check(hardblock *parent);
int Calculate_HPWL(net **nets_arr, int NETS_NUM, hardblock **blks_arr, tml **tmls_arr);
