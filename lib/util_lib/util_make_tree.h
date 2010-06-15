NT_node ** seq2cw_tree ( Sequence *S, char *file);
NT_node ** make_nj_tree (  Alignment *A,int **distances,int gop, int gep, char **out_seq, char **out_seq_name, int out_nseq, char *tree_file, char *tree_mode);
NT_node ** make_upgma_tree (  Alignment *A,int **distances,int gop, int gep, char **out_seq, char **out_seq_name, int out_nseq, char *tree_file, char *tree_mode);

NT_node ** int_dist2nj_tree (int **distances, char **out_seq_name, int out_nseq,  char *tree_file);
NT_node ** float_dist2nj_tree (float **distances, char **out_seq_name, int out_nseq,  char *tree_file);
NT_node ** dist2nj_tree (double **distances, char **out_seq_name, int out_nseq,  char *tree_file);

NT_node ** int_dist2upgma_tree (int **mat, Alignment *A, int nseq, char *fname);
NT_node upgma_merge (int **mat, NT_node *NL, int *used, int *n, int N);

void nj_tree(char **tree_description, int nseq);
void fast_nj_tree(char **tree_description);
void slow_nj_tree(char **tree_description);

void print_phylip_tree(char **tree_description, FILE *tree, int bootstrap);
void two_way_split(char **tree_description, FILE *tree, int start_row, int flag, int bootstrap);
void guide_tree(char *fname, double **saga_tmat, char **sag_seq_name, int saga_nseq);



NT_node split2upgma_tree (Split **S, Alignment *A, int nseq, char *fname);
NT_node split_upgma_merge (Alignment *A, Split **S, NT_node *NL, int *used, int *n, int N);
float get_split_dist ( Alignment *A, NT_node L, NT_node R, Split **S) ;

Alignment * upgma_tree_aln  (Alignment*A, int nseq, Constraint_list *CL);
int ** dist_mat2best_split (int **mat, int nseq);
int upgma_node_heap (NT_node X);