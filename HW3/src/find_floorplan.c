#include "main.h"
#include "find_floorplan.h"

void Orient_Correction(int BLKS_NUM, hardblock **blks_arr){
    int cnt = 0, swp;
    while(cnt<BLKS_NUM){
        if(blks_arr[cnt]->height < blks_arr[cnt]->width){
            swp = blks_arr[cnt]->height;
            blks_arr[cnt]->height = blks_arr[cnt]->width;
            blks_arr[cnt]->width = swp;

            blks_arr[cnt]->rotate = true;
        }
        cnt++;
    }
}

void Create_Init_Bstree(int BLKS_NUM, hardblock **blks_arr, sort_node *sorted_list){
    sort_node *curr_node;
    int cnt = 0;
    sort_node **sorted_node;

    sorted_node = malloc(BLKS_NUM*sizeof(sort_node *));

    /*  list to array   */
    curr_node = sorted_list;
    while(curr_node!=NULL){
        sorted_node[cnt] = curr_node;
        /*printf("%d\n", sorted_node[cnt]->num);*/

        curr_node = curr_node->next;
        sorted_node[cnt]->next = NULL;
        cnt++;
    }    

    /*  build B* tree   */
    cnt = 0;
    while((2*cnt+1)<BLKS_NUM){
        if((2*cnt+1)<BLKS_NUM) blks_arr[sorted_node[cnt]->num]->left_child = blks_arr[sorted_node[2*cnt+1]->num];
        if((2*cnt+2)<BLKS_NUM) blks_arr[sorted_node[cnt]->num]->right_child = blks_arr[sorted_node[2*cnt+2]->num];

        cnt++;
    }
}

sort_node *Sort_Blks(int BLKS_NUM, int W, int H, hardblock **blks_arr){
    sort_node **sorted_blks;
    sort_node *new_node, *list = NULL, *tmp_blk, *next_blk;
    int blks_cnt = 0, node_cnt = 0;

    sorted_blks = malloc((W+1)*sizeof(sort_node *));

    /*  initial sorted_blks array   */
    while(node_cnt<W+1){
        sorted_blks[node_cnt] = NULL;
        node_cnt++;
    }

    /*  sort blocks with width  */
    while(blks_cnt<BLKS_NUM){
        /*  new node    */
        new_node = malloc(sizeof(sort_node));
        new_node->num = blks_arr[blks_cnt]->num;
        new_node->next = NULL;
        /*printf("%d\n", blks_arr[blks_cnt]->num);*/

        /*  insert into array   */
        if(sorted_blks[blks_arr[blks_cnt]->width]!=NULL) new_node->next = sorted_blks[blks_arr[blks_cnt]->width];
        sorted_blks[blks_arr[blks_cnt]->width] = new_node;

        blks_cnt++;
    }

    /*  link block-list */
    node_cnt = W;
    while(node_cnt>=0){
        if(sorted_blks[node_cnt] == NULL){
            node_cnt--;
            continue;
        } 

        if(list != NULL){
            tmp_blk = sorted_blks[node_cnt];
            while(tmp_blk->next!=NULL){                
                tmp_blk = tmp_blk->next;
            }  

            tmp_blk->next = list;        
        }
        list = sorted_blks[node_cnt];

        node_cnt--;
    }

    /*  clean sorted_blks array */
    while(node_cnt<W+1){
        sorted_blks[node_cnt] = NULL;
        node_cnt++;
    }

    /*  sort blocks with height */
    while(list!=NULL){
        /*  insert into array   */
        next_blk = list->next;

        list->next = NULL;
        if(sorted_blks[blks_arr[list->num]->height]!=NULL) list->next = sorted_blks[blks_arr[list->num]->height];
        sorted_blks[blks_arr[list->num]->height] = list;

        list = next_blk;
    }

    /*  link block-list */ 

    list = NULL;   

    node_cnt = 0;
    while(node_cnt<=H){
        if(sorted_blks[node_cnt] == NULL){
            node_cnt++;
            continue;
        } 

        if(list != NULL){
            tmp_blk = sorted_blks[node_cnt];
            while(tmp_blk->next!=NULL){                
                tmp_blk = tmp_blk->next;
            }  

            tmp_blk->next = list;        
        }
        list = sorted_blks[node_cnt];

        node_cnt++;
    }

    return list;
}

void Init_Floorplan(hardblock *parent, int *contour, int W){
    /*printf("%d %d %d\n", parent->num, parent->cor_x, parent->cor_y);*/
    if(parent->left_child!=NULL){
        parent->left_child->cor_x = parent->cor_x;   
        parent->left_child->cor_y = parent->cor_y + parent->height;        
        Init_Floorplan(parent->left_child, contour, W);
    }

    if(parent->right_child!=NULL){
        parent->right_child->cor_x = parent->cor_x + parent->width;
        parent->right_child->cor_y = parent->cor_y;
        Init_Floorplan(parent->right_child, contour, W);
    }
}

bool Sorted_Floorplan(int BLKS_NUM, int W, int H, hardblock **blks_arr, sort_node *sorted_list){
    int curr_x = 0, curr_y = 0, next_y;
    sort_node *curr_node = sorted_list;
    bool out_of_bound = false;    

    next_y = curr_y + blks_arr[curr_node->num]->height;
    while(curr_node!=NULL){
        if((curr_x + blks_arr[curr_node->num]->width) > W){
            curr_x = 0;
            curr_y = next_y;            
            next_y = curr_y + blks_arr[curr_node->num]->height;

            if(next_y > H) out_of_bound = true; 
        }

        /*printf("%d %d %d %d | ", blks_arr[curr_node->num]->num, blks_arr[curr_node->num]->height, blks_arr[curr_node->num]->width, blks_arr[curr_node->num]->rotate);*/

        blks_arr[curr_node->num]->cor_x = curr_x;
        curr_x = curr_x + blks_arr[curr_node->num]->width;

        blks_arr[curr_node->num]->cor_y = curr_y;

        /* printf("%d %d\n", blks_arr[curr_node->num]->cor_x , blks_arr[curr_node->num]->cor_y); */

        curr_node = curr_node->next;        
    }

    return out_of_bound;
}

void Flip_Sorted_Floorplan(hardblock **blks_arr, sort_node *sorted_list, int W, int H){    
    sort_node *curr_node;
    bool flip = true;
    int curr_x;

    /*  Flip    */
    curr_node = sorted_list;
    while(curr_node!=NULL){
        if(blks_arr[curr_node->num]->cor_x==0){
            flip = !flip;
            curr_x = W; 
        }
        
        /* printf("%d\n", flip); */
        if(flip){
            blks_arr[curr_node->num]->cor_x = curr_x - blks_arr[curr_node->num]->width;
            curr_x = blks_arr[curr_node->num]->cor_x;
        }

        curr_node = curr_node->next;
    }
}

void Globally_Compact(hardblock **blks_arr, sort_node *sorted_list, int *contour, int W, int H){
    sort_node *curr_node;
    int i, curr_top;

    /*  Globally Compact    */
    curr_node = sorted_list;
    while(curr_node!=NULL){
        /*  Bottom Compact  */
        curr_top = -100;

        for(i=blks_arr[curr_node->num]->cor_x; i<blks_arr[curr_node->num]->cor_x+blks_arr[curr_node->num]->width; i++){
            if(curr_top < contour[i]) curr_top = contour[i];
        }

        blks_arr[curr_node->num]->cor_y = curr_top;

        /*  Make Contour    */    
        for(i=blks_arr[curr_node->num]->cor_x; i<blks_arr[curr_node->num]->cor_x+blks_arr[curr_node->num]->width; i++){
            contour[i] = blks_arr[curr_node->num]->cor_y + blks_arr[curr_node->num]->height;
        }    

        curr_node = curr_node->next;
    }
}

#define max(a, b) (a>b)?a:b
#define min(a, b) (a<b)?a:b

void Row_Based_Adjust(hardblock **blks_arr, sort_node *sorted_list, int W, int H){
    sort_node *curr_node, *selected_node, *row_head, *row_search;
    int curr_hpwl, selected_hpwl, swp_x;
    int o_curr_hpwl, o_selected_hpwl;
    int adjust, left_blk, right_blk;
    list_node *curr_lnode;   
    int T, alpha;
    T = 15000;
    alpha = 0.5;

    /*  Don't Flip  */
    /*  Same Row    */
    while(T>10){
    curr_node = sorted_list;
    row_head = sorted_list;
    while(curr_node!=NULL){
        /* printf("scan\n"); */
        if(blks_arr[curr_node->num]->cor_y != blks_arr[row_head->num]->cor_y) row_head = curr_node;/*change row*/  
        /* printf("curr_node->num : %d\n", curr_node->num);       */

        selected_node = curr_node->next;
        while(selected_node!=NULL){
            if(blks_arr[selected_node->num]->cor_y != blks_arr[curr_node->num]->cor_y) break;

            /*  swap cor_x  */
            /* printf("swap cor_x\n"); */
            swp_x = blks_arr[curr_node->num]->cor_x;
            blks_arr[curr_node->num]->cor_x = blks_arr[selected_node->num]->cor_x;
            blks_arr[selected_node->num]->cor_x = swp_x;            

            selected_hpwl = 0;
            o_selected_hpwl = 0;
            curr_hpwl = 0;
            o_curr_hpwl = 0;

            /* printf("selected_node->num : %d\n", selected_node->num); */
            /*  find new HPWL   */
            /* printf("find new HPWL\n"); */
            curr_lnode = blks_arr[selected_node->num]->pin_list;
            while(curr_lnode!=NULL){
                selected_hpwl += get_hpwl(curr_lnode->ptr.n_ptr);
                o_selected_hpwl += curr_lnode->ptr.n_ptr->hpwl;
                curr_lnode = curr_lnode->next;
            }

            curr_lnode = blks_arr[curr_node->num]->pin_list;
            while(curr_lnode!=NULL){
                curr_hpwl += get_hpwl(curr_lnode->ptr.n_ptr);
                o_curr_hpwl += curr_lnode->ptr.n_ptr->hpwl;
                curr_lnode = curr_lnode->next;
            }

            /*  adjust cor_x    */  /*  SA accept incease HPWL  */
            /* printf("adjust cor_x\n"); */
            if(curr_hpwl<o_curr_hpwl && selected_hpwl<o_selected_hpwl){
                /*  update the cor_x of each block in the row   */
                if(blks_arr[curr_node->num]->cor_x > blks_arr[selected_node->num]->cor_x) adjust = blks_arr[curr_node->num]->width - blks_arr[selected_node->num]->width;
                else adjust = blks_arr[selected_node->num]->width - blks_arr[curr_node->num]->width;

                left_blk = min(blks_arr[curr_node->num]->cor_x, blks_arr[selected_node->num]->cor_x);
                right_blk = max(blks_arr[curr_node->num]->cor_x, blks_arr[selected_node->num]->cor_x);

                /*  search sorted_list from the row head to the row end   */
                row_search = row_head;
                while(row_search!=NULL){
                    if(blks_arr[row_search->num]->cor_y != blks_arr[row_head->num]->cor_y) break;

                    /*  find blocks which is on the righthandside of these two blocks and adjust cor_xs of these blocks*/
                    if(blks_arr[row_search->num]->cor_x>left_blk && blks_arr[row_search->num]->cor_x<=right_blk){
                        blks_arr[row_search->num]->cor_x -= adjust;
                    }

                    row_search = row_search->next;
                }             
            }
            else{
                /*  swap back   */
                swp_x = blks_arr[curr_node->num]->cor_x;
                blks_arr[curr_node->num]->cor_x = blks_arr[selected_node->num]->cor_x;
                blks_arr[selected_node->num]->cor_x = swp_x;
            }
            /* printf("done in\n"); */
            selected_node = selected_node->next;
        }

        /* printf("done out\n"); */
        curr_node = curr_node->next;
    }
    T--;
    }
}

int get_hpwl(net *nets_ptr){
    int top, bottom, left, right;
    int hpwl = 0, pin_x, pin_y;
    list_node *curr_node;

    top = -100000;
    bottom = 100000;
    left = 100000;
    right = -100000;

    curr_node = nets_ptr->pin_list;
    /* printf("nets_cnt : %d\n", nets_cnt); */
    while(curr_node!=NULL){
        /* printf("%d\n", curr_node->num); */
        if(curr_node->type==TYPE_TML){
            /* printf("TYPE_TML\n"); */
            if(top<curr_node->ptr.t_ptr->cor_y) top = curr_node->ptr.t_ptr->cor_y;
            if(bottom>curr_node->ptr.t_ptr->cor_y) bottom = curr_node->ptr.t_ptr->cor_y;
            if(left>curr_node->ptr.t_ptr->cor_x) left = curr_node->ptr.t_ptr->cor_x;
            if(right<curr_node->ptr.t_ptr->cor_x) right = curr_node->ptr.t_ptr->cor_x;
        }
        else if(curr_node->type==TYPE_BLK){
            /* printf("TYPE_BLK\n"); */
            pin_x = curr_node->ptr.b_ptr->cor_x + curr_node->ptr.b_ptr->width/2;
            pin_y = curr_node->ptr.b_ptr->cor_y + curr_node->ptr.b_ptr->height/2;

            if(top<pin_y) top = pin_y;
            if(bottom>pin_y) bottom = pin_y;                
            if(left>pin_x) left = pin_x;
            if(right<pin_x) right = pin_x;
        }

        curr_node = curr_node->next;
    }

    hpwl = (top-bottom)+(right-left);

    return hpwl;
}

sort_node *Row_Change(hardblock **blks_arr, sort_node *sorted_list, int W, int H, net **nets_arr, int NETS_NUM, tml **tmls_arr){
    sort_node *curr_node, *next_node, *final_list, *swp, *last_node;
    sort_node **row_arr;
    int row_num = 0, row_cnt = 1;
    int curr_row = 0, selected_row = 0, curr_y, selected_y;
    int o_hpwl, new_hpwl;
    int adjust, top, bottom;
    int i;    
    
    curr_node = sorted_list;
    while(curr_node!=NULL){
        if(blks_arr[curr_node->num]->cor_x == 0) row_num++;
        curr_node = curr_node->next;
    }

    row_arr = malloc(row_num*sizeof(sort_node *));
    curr_node = sorted_list;
    last_node = sorted_list;
    row_arr[0] = sorted_list;
    while(curr_node!=NULL){
        if(blks_arr[curr_node->num]->cor_y != blks_arr[last_node->num]->cor_y){
            row_arr[row_cnt] = curr_node;
            row_cnt++;
            last_node->next = NULL;
        }

        last_node = curr_node;
        curr_node = curr_node->next;
    }   

    while(curr_row<row_num){
        selected_row = curr_row+1;
        while(selected_row<row_num){
            o_hpwl = Calculate_HPWL(nets_arr, NETS_NUM, blks_arr, tmls_arr);
            /*  row change  */
            curr_y = blks_arr[row_arr[curr_row]->num]->cor_y;
            selected_y = blks_arr[row_arr[selected_row]->num]->cor_y;
            
            /*  update cor_y    */
            curr_node = row_arr[curr_row];
            while(curr_node!=NULL){
                blks_arr[curr_node->num]->cor_y = selected_y;
                curr_node = curr_node->next;
            }

            curr_node = row_arr[selected_row];
            while(curr_node!=NULL){
                blks_arr[curr_node->num]->cor_y = curr_y;
                curr_node = curr_node->next;
            }

            /*  calculate HPWL  */
            new_hpwl = Calculate_HPWL(nets_arr, NETS_NUM, blks_arr, tmls_arr);

            /*  keep or redo    */
            if(new_hpwl>o_hpwl){
                /*  redo    */
                curr_node = row_arr[curr_row];
                while(curr_node!=NULL){
                    blks_arr[curr_node->num]->cor_y = curr_y;
                    curr_node = curr_node->next;
                }

                curr_node = row_arr[selected_row];
                while(curr_node!=NULL){
                    blks_arr[curr_node->num]->cor_y = selected_y;
                    curr_node = curr_node->next;
                }
            }
            else{
                /*  really move */
                if(blks_arr[row_arr[curr_row]->num]->cor_y > blks_arr[row_arr[selected_row]->num]->cor_y) adjust = blks_arr[row_arr[curr_row]->num]->height - blks_arr[row_arr[selected_row]->num]->height;
                else adjust = blks_arr[row_arr[selected_row]->num]->height - blks_arr[row_arr[curr_row]->num]->height;

                top = max(blks_arr[row_arr[curr_row]->num]->cor_y, blks_arr[row_arr[selected_row]->num]->cor_y);
                bottom = min(blks_arr[row_arr[curr_row]->num]->cor_y, blks_arr[row_arr[selected_row]->num]->cor_y);

                for(i=0; i<row_cnt; i++){
                    if(blks_arr[row_arr[i]->num]->cor_y>bottom && blks_arr[row_arr[i]->num]->cor_y<=top){
                        curr_node = row_arr[i];
                        while(curr_node!=NULL){
                            blks_arr[curr_node->num]->cor_y -= adjust;
                            curr_node = curr_node->next;
                        }
                    }
                }

                /*  revise array    */
                swp = row_arr[curr_row];
                row_arr[curr_row] = row_arr[selected_row];
                row_arr[selected_row] = swp;
            }

            selected_row++;
        }

        curr_row++;
    }    

    /*  build final_list    */    
    for(i=0; i<row_num-1; i++){
        curr_node = row_arr[i];
        while(curr_node->next!=NULL){
            curr_node = curr_node->next;
        }
        curr_node->next = row_arr[i+1];
    }
    final_list = row_arr[0];

    return final_list;
}