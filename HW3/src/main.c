#include "main.h"
#include "read_input.h"
#include "find_floorplan.h"

int main(int argc, char *argv[]){
    FILE *blocks_file, *nets_file, *pl_file, *out_file;
    hardblock **blks_arr;
    tml **tmls_arr;
    net **nets_arr;
    sort_node *sorted_list, *final_list;
    int *contour;

    int i;
    bool out_of_bound = false;

    int BLKS_NUM = 0, TMLS_NUM = 0;
    int NETS_NUM = 0, PINS_NUM = 0;    
    int WIRELENGTH = 0;
    int AREA = 0;

    float DEAD_RATIO = 0;
    float W = 0.0, H = 0.0;

    bool flag;

    if(argc != 6){
		printf("\n Error. Takes 5 arguments\n");
		return -1;
	}

    DEAD_RATIO = atof(argv[5]);
    printf("DEAD RATIO : %.2f\n", DEAD_RATIO);

    blocks_file = R_file(argv[1], "r");
    if(blocks_file == NULL) return -1;
    
    nets_file = R_file(argv[2], "r");
    if(nets_file == NULL) return -1;

    pl_file = R_file(argv[3], "r");
    if(pl_file == NULL) return -1;

    out_file = R_file(argv[4], "w");
    if(out_file == NULL) return -1;

	printf("Open files\n");
    
    fscanf(blocks_file, "NumHardRectilinearBlocks : %d\n", &BLKS_NUM);
    fscanf(blocks_file, "NumTerminals : %d\n", &TMLS_NUM);

    printf("NumHardRectilinearBlocks : %d\n", BLKS_NUM);
    printf("NumTerminals : %d\n", TMLS_NUM);

    blks_arr = malloc(BLKS_NUM*sizeof(hardblock *));
    tmls_arr = malloc((TMLS_NUM+1)*sizeof(tml *));

    printf("build_hardblocks\n");
    build_hardblocks(blocks_file, blks_arr, tmls_arr, BLKS_NUM, &AREA);
    printf("Area : %d, %.2f\n", AREA, AREA*(1+DEAD_RATIO));

    W = sqrt(AREA*(1+DEAD_RATIO));
    H = W;
    printf("W : %.2f, H : %.2f\n", W, H);

    contour = malloc((W+1)*sizeof(int));
    for(i=0; i<=W; i++) contour[i] = 0;


    fscanf(nets_file, "NumNets : %d\n", &NETS_NUM);
    fscanf(nets_file, "NumPins : %d\n", &PINS_NUM);

    printf("NumNets : %d\n", NETS_NUM);
    printf("NumPins : %d\n", PINS_NUM);

    printf("build_net\n");
    nets_arr = malloc(NETS_NUM*sizeof(net *));
    build_net(nets_file, nets_arr, blks_arr, tmls_arr);

    printf("build_pin\n");
    build_pin(pl_file, tmls_arr);

    /*Pins_Num_Check(BLKS_NUM, TMLS_NUM, NETS_NUM, PINS_NUM, blks_arr, tmls_arr, nets_arr);*/

    /*  initial floorplan   */
    printf("Orient Correction\n");
    Orient_Correction(BLKS_NUM, blks_arr);

    printf("Sort_Blks\n");
    sorted_list = Sort_Blks(BLKS_NUM, W, H, blks_arr);
    /*Sorted_List_Check(sorted_list, BLKS_NUM, blks_arr);*/

    printf("Sorted Floorplan\n");
    out_of_bound = Sorted_Floorplan(BLKS_NUM, W, H, blks_arr, sorted_list);
    if(out_of_bound) printf("Sorted Floorplan out of bound\n");    

    /*printf("Create_Init_Bstree\n");
    Create_Init_Bstree(BLKS_NUM, blks_arr, sorted_list);
    printf("Traverse_Bstree_Check\n");*/
    /*Traverse_Bstree_Check(blks_arr[sorted_list->num]);*/

    /*  Initial floorplan   */
    /*for(i=0; i<2*W; i++) contour[i] = -1;
    Init_Floorplan(blks_arr[sorted_list->num], contour, W);*/

    /*  Find Temp HPWL  */
    WIRELENGTH = Calculate_HPWL(nets_arr, NETS_NUM, blks_arr, tmls_arr);

    flag = false;
    if(flag){
        printf("Flip Sorted Floorplan\n");
        Flip_Sorted_Floorplan(blks_arr, sorted_list, W, H); 
    }
    else{       
        printf("Row based Adjust\n");
        Row_Based_Adjust(blks_arr, sorted_list, W, H);
    }
        
    /*  Different Row but same width    */

    /*  Row Change  */
    final_list = Row_Change(blks_arr, sorted_list, W, H, nets_arr, NETS_NUM, tmls_arr);
    /* final_list = sorted_list; */

    /*  Simulating Anealing to Find Minimum HPWL    */

    /*  Globally Compact    */
    printf("Globally Compact\n");
    Globally_Compact(blks_arr, final_list, contour, W, H);

    /*  Calculate Wirelength    */
    printf("Calculate HPWL\n");
    WIRELENGTH = Calculate_HPWL(nets_arr, NETS_NUM, blks_arr, tmls_arr);    
    printf("HPWL : %d\n", WIRELENGTH);

    /*  write file  */
    printf("Write Output File\n");
    W_file(out_file, WIRELENGTH, BLKS_NUM, blks_arr);
    Draw_PNG(BLKS_NUM, blks_arr, W, H);

    fclose(blocks_file);
    fclose(nets_file);
    fclose(pl_file);
    fclose(out_file);

    return 0;
}

FILE *R_file(char *file, char *mode){
	FILE *f;
	
	f = fopen(file, mode);
	if(f == NULL){
		printf("Unable to open %s.\n", file);		
		return NULL;
	}

	printf("Open %s successfully.\n", file);
	return f;
}

void W_file(FILE *f, int WIRELENGTH, int BLKS_NUM, hardblock **blks_arr){
    int i;

    fprintf(f, "Wirelength %d\n", WIRELENGTH);
    fprintf(f, "Blocks\n");

    for(i=0; i<BLKS_NUM; i++){
        fprintf(f, "sb%d %d %d %d\n", blks_arr[i]->num, blks_arr[i]->cor_x, blks_arr[i]->cor_y, blks_arr[i]->rotate);
    }
}

void Draw_PNG(int BLKS_NUM, hardblock **blks_arr, int W, int H){
    int i;
    float alpha_c;
    time_t t;

    srand((unsigned) time(&t));

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1.3*W, 1.3*H);
    cairo_t *outline = cairo_create(surface);
    cairo_t **cr;

    cr = malloc(BLKS_NUM*sizeof(cairo_t *));

    cairo_set_source_rgba(outline, 1, 1, 1, 0.01);
    cairo_set_line_width(outline, 60);

    cairo_rectangle(outline, 0, 0, W, H);
    cairo_fill(outline);


    for(i=0; i<BLKS_NUM; i++){
        cr[i] = cairo_create(surface);

        alpha_c = i%10+1;
        /* printf("%0.2f\n", alpha_c/10); */

        cairo_set_source_rgba(cr[i], 1, 0, 0, alpha_c/10);        
        cairo_set_line_width(cr[i], 60);
        
        cairo_rectangle(cr[i], blks_arr[i]->cor_x, blks_arr[i]->cor_y, blks_arr[i]->width, blks_arr[i]->height);
        cairo_fill(cr[i]);
    };    
    
    cairo_surface_write_to_png(surface, "../output/result.png");
    cairo_surface_destroy(surface);
}

void Pins_Num_Check(int BLKS_NUM,int TMLS_NUM, int NETS_NUM, int PINS_NUM, hardblock **blk_arr, tml **tml_arr, net **net_arr){
    int i, j, pins_cnt = 0, np_cnt = 0;
    for(i=0; i<BLKS_NUM; i++){
        pins_cnt += blk_arr[i]->pins_cnt;
    }

    for(i=1; i<=TMLS_NUM; i++){
        pins_cnt += tml_arr[i]->pins_cnt;
    }

    for(j=0; j<NETS_NUM; j++){
        np_cnt += net_arr[j]->pins_cnt;
    }

    printf("%d, %d, %d\n", pins_cnt, np_cnt, PINS_NUM);
}

void Sorted_List_Check(sort_node *list, int BLKS_NUM, hardblock **blks_arr){
    sort_node *curr_node;
    int cnt = 0;

    curr_node = list;
    while(curr_node!=NULL){
        printf("%d %d %d %d\n", blks_arr[curr_node->num]->num, blks_arr[curr_node->num]->height, blks_arr[curr_node->num]->width, blks_arr[curr_node->num]->rotate);
        curr_node = curr_node->next;
        cnt++;
    }

    printf("%d %d\n", BLKS_NUM, cnt);
}

void Traverse_Bstree_Check(hardblock *parent){
    printf("%d ", parent->num);
    if(parent->left_child != NULL) printf("%d ", parent->left_child->num);
    if(parent->right_child != NULL) printf("%d ", parent->right_child->num);
    printf("\n");

    if(parent->left_child != NULL) Traverse_Bstree_Check(parent->left_child);
    if(parent->right_child != NULL) Traverse_Bstree_Check(parent->right_child);
}

int Calculate_HPWL(net **nets_arr, int NETS_NUM, hardblock **blks_arr, tml **tmls_arr){
    int nets_cnt = 0;
    int top, bottom, left, right;
    int HPWL = 0, pin_x, pin_y;
    int pin_cnt = 0;
    list_node *curr_node;

    while(nets_cnt < NETS_NUM){        
        if(nets_arr[nets_cnt]->pin_list==NULL){
            nets_cnt++;
            continue;
        }

        top = -100000;
        bottom = 100000;
        left = 100000;
        right = -100000;

        curr_node = nets_arr[nets_cnt]->pin_list;
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
            pin_cnt++;
        }

        /* printf("%d %d %d %d\n", top, bottom, right, left); */

        nets_arr[nets_cnt]->hpwl = (top-bottom)+(right-left);
        HPWL += nets_arr[nets_cnt]->hpwl;

        nets_cnt++;
    }

    /* printf("nets_cnt : %d, pin_cnt : %d\n", nets_cnt, pin_cnt); */
    printf("HPWL : %d\n", HPWL);

    return HPWL;
}