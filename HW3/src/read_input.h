void build_hardblocks(FILE *f, hardblock **blks_arr, tml **tmls_arr, int blks_num, int *t_area);
void build_net(FILE *f, net **nets_arr, hardblock **blks_arr, tml **tmls_arr);
void build_pin(FILE *f, tml **tmls_arr);