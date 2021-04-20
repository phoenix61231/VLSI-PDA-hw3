#include "main.h"
#include "read_input.h"

void build_hardblocks(FILE *f, hardblock **blks_arr, tml **tmls_arr, int blks_num, int *t_area){
    hardblock *new_block;
    tml *new_tml;    
    int blk_num = 0, tml_num = 0;
    int lb_x, lb_y, rt_x, rt_y;
    int lb_t, rt_t;

    while(!feof(f)){             
        fscanf(f, "sb%d hardrectilinear 4 (%d, %d) (0, %d) (%d, %d) (%d, 0)\n", &blk_num, &lb_x, &lb_y, &lb_t, &rt_x, &rt_y, &rt_t);
        /*printf("%d\n", blk_num);*/
        
        new_block = malloc(sizeof(hardblock));
        blks_arr[blk_num] = new_block;

        new_block->num = blk_num;
        new_block->cor_x = lb_x;
        new_block->cor_y = lb_y;
        new_block->height = rt_y;
        new_block->width = rt_x;
        new_block->rotate = false;
        new_block->left_child = NULL;
        new_block->right_child = NULL;

        new_block->pins_cnt = 0;
        new_block->pin_list = NULL;        

        new_block->next = NULL;
        new_block->prev = NULL;

        *t_area += rt_x*rt_y;

        if(blk_num >= blks_num-1) break;
    }
    
    tmls_arr[0] = NULL;
    while(!feof(f)){
        fscanf(f, "p%d terminal\n", &tml_num);
        /*printf("p%d terminal\n", tml_num);*/

        new_tml = malloc(sizeof(tml));
        tmls_arr[tml_num] = new_tml;

        new_tml->num = tml_num;
        new_tml->pins_cnt = 0;
        new_tml->cor_x = 0;
        new_tml->cor_y = 0;
        
        new_tml->next = NULL;
        new_tml->prev = NULL;
    }
}

void build_net(FILE *f, net **nets_arr, hardblock **blks_arr, tml **tmls_arr){    
    int deg_num = 0, tml_num = 0, blk_num = 0;
    net *new_net;
    int net_cnt = 0, pin_cnt = 0;
    list_node *new_node;

    while(!feof(f)){
        /*  net form    */
        new_net = malloc(sizeof(net));
        new_net->num = net_cnt;

        fscanf(f, "NetDegree : %d\n", &deg_num);
        /*printf("NetDegree : %d\n", deg_num);*/
        new_net->pins_cnt = deg_num;

        pin_cnt = 0;
        new_net->pin_list = NULL;       
        
        while(pin_cnt < deg_num){
            /*  insert net list */

            if(fscanf(f, "p%d\n", &tml_num)!=0){
                /*printf("p%d\n", tml_num);*/
                /*  insert net list */
                new_node  = malloc(sizeof(list_node));
                new_node->type = TYPE_TML;
                new_node->num = tml_num;
                new_node->ptr.t_ptr = tmls_arr[tml_num];
                new_node->next = NULL;
                if(new_net->pin_list != NULL) new_node->next = new_net->pin_list; 
                new_net->pin_list = new_node;

                /*  tml pins_cnt update */
                tmls_arr[tml_num]->pins_cnt+=1;
            
                pin_cnt++;
            }            
            else if(fscanf(f, "sb%d\n", &blk_num)!=0){
                /*printf("sb%d\n", blk_num);*/
                new_node  = malloc(sizeof(list_node));
                new_node->type = TYPE_BLK;
                new_node->num = blk_num;
                new_node->ptr.b_ptr = blks_arr[blk_num]; 
                new_node->next = NULL;           
                if(new_net->pin_list != NULL) new_node->next = new_net->pin_list; 
                new_net->pin_list = new_node;

                /*  insert blk list */
                blks_arr[blk_num]->pins_cnt+=1;
                new_node  = malloc(sizeof(list_node));
                new_node->type = TYPE_NET;
                new_node->num = net_cnt;
                new_node->ptr.n_ptr = new_net; 
                new_node->next = NULL;           
                if(blks_arr[blk_num]->pin_list != NULL) new_node->next = blks_arr[blk_num]->pin_list; 
                blks_arr[blk_num]->pin_list = new_node;

                pin_cnt++; 
            }                       
        }

        new_net->hpwl = 0;

        new_net->next = NULL;
        new_net->prev = NULL;

        nets_arr[net_cnt] = new_net;
        net_cnt++;
        /*printf("%d\n", net_cnt);*/
    }
}

void build_pin(FILE *f, tml **tmls_arr){
    int tml_num, cor_x, cor_y;

    while(!feof(f)){
        fscanf(f, "p%d %d %d\n", &tml_num, &cor_x, &cor_y);
        /*printf("p%d %d %d\n", tml_num, cor_x, cor_y);*/
        
        tmls_arr[tml_num]->cor_x = cor_x;
        tmls_arr[tml_num]->cor_y = cor_y;
    }
}