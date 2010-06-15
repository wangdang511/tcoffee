//DIR_4_TCOFFEE [def: ~/.t_coffee]: UNIQUE_DIR_4_TCOFFEE -> DIR_4_TCOFFEE ->HOME/.t_coffee
//TMP_4_TCOFFEE [def: ~/.t_coffee/tmp]:: UNIQUE_DIR_4_TCOFFEE -> TMP_4_TCOFFEE ->DIR_4_TCOFFEE/tmp

#define FILE_CHECK 1
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/file.h>


#include "io_lib_header.h"
#include "util_lib_header.h"
#include "define_header.h"
#include "perl_header_lib.h"


int my_vsscanf(char *buf, char *fmt, va_list parms);
static int get_vtmpnam2_root();




static int global_exit_signal;
static int no_error_report;
static int clean_exit_started;
static int debug_lock;
static char *in_cl;
static char *logfile;
/*********************************************************************/
/*                                                                   */
/*                                  DICHOTOMY                        */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
double dichotomy (double value, double target_value, double middle, double *bottom,double *top)
{
  if ( value> target_value)top[0]=middle;
  else if ( value<target_value)bottom[0]=middle;
  return (top[0]-bottom[0])/2+bottom[0];
}


/*********************************************************************/
/*                                                                   */
/*                                   QSORT                           */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#define MAXSTACK (sizeof(size_t) * CHAR_BIT)
static void exchange(void *a, void *b, size_t size) {
    size_t i;
    int *ia;
    int *ib;
    char *ca;
    char *cb;
    
    /******************
     *  exchange a,b  *
     ******************/
    ia=(int*)a;
    ib=(int*)b;
    for (i = sizeof(int); i <= size; i += sizeof(int)) {
        int t =ia[0];
	ia[0]=ib[0];
	ib[0]=t;
	ia++; ib++;
    
    }
    ca=(char*)ia;
    cb=(char*)ib;
    for (i = i - sizeof(int) + 1; i <= size; i++) {
        
        char t=ca[0];
	ca[0]=cb[0];
	cb[0]=t;
	ca++;cb++;

    }
}

void qsort(void *base, size_t nmemb, size_t size,
        int (*compar)(const void *, const void *)) {
    void *lbStack[MAXSTACK], *ubStack[MAXSTACK];
    int sp;
    unsigned int offset;

    /********************
     *  ANSI-C qsort()  *
     ********************/

    lbStack[0] = (char *)base;
    ubStack[0] = (char *)base + (nmemb-1)*size;
    for (sp = 0; sp >= 0; sp--) {
        char *lb, *ub, *m;
        char *P, *i, *j;

        lb = lbStack[sp];
        ub = ubStack[sp];

        while (lb < ub) {

            /* select pivot and exchange with 1st element */
            offset = (ub - lb) >> 1;
            P = lb + offset - offset % size;
            exchange (lb, P, size);

            /* partition into two segments */
            i = lb + size;
            j = ub;
            while (1) {
                while (i < j && compar(lb, i) > 0) i += size;
                while (j >= i && compar(j, lb) > 0) j -= size;
                if (i >= j) break;
                exchange (i, j, size);
                j -= size;
                i += size;
            }

            /* pivot belongs in A[j] */
            exchange (lb, j, size);
            m = j;

            /* keep processing smallest segment, and stack largest */
            if (m - lb <= ub - m) {
                if (m + size < ub) {
                    lbStack[sp] = m + size;
                    ubStack[sp++] = ub;
                }
                ub = m - size;
            } else {
                if (m - size > lb) {
                    lbStack[sp] = lb; 
                    ubStack[sp++] = m - size;
                }
                lb = m + size;
            }
        }
    }
}

int pstrcmp(char *p1, char *p2);



/*********************************************************************/
/*                                                                   */
/*                                   HEAPSORT                           */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
FILE * hsort_file ( FILE *fp,int n,int len, size_t size,int first_comp_field, int n_comp_fields,int (*compare)(const void *, const void*,int, int, size_t),void * (*copy)(void *,void*, size_t))
     {
     unsigned long i, ir, j, l;
     void *rra, *rrb, *ra_j, *ra_j_1;
     void *tp;
     long  start;
     int FE=1;
     
    
    
     start=ftell(fp);
     rra   =vcalloc ( len, size);
     rrb   =vcalloc ( len, size);
     ra_j  =vcalloc ( len, size);
     ra_j_1=vcalloc ( len, size);

     if ( n<2)return fp;
     l=(n >>1)+1;
     ir=n;

     for (;;)
         {
	 if ( l>FE)
	    {
	    l--;
	    fseek( fp, start+(((l-1)*len)*size), SEEK_SET);
	    fread( rra, size, len,fp); /*rra=ra[--l]*/
	    }
	 else
	    {
	    fseek( fp, start+((ir-1)*len*size), SEEK_SET);
	    fread( rra, size, len,fp); /*rra=ra[ir]*/
	    
	    fseek( fp, start, SEEK_SET);
	    fread( rrb, size, len,fp); /*rrb=ra[0]*/
	    
	    fseek( fp, start+((ir-1)*len*size), SEEK_SET);
	    fwrite(rrb,size, len, fp); /*ra[ir]=rrb=ra[0]*/ 

	    if (--ir ==FE)
	       {
	       fseek ( fp,start, SEEK_SET);
	       fwrite(rra,size, len, fp); /*ra[0]=rra*/ 
	       break;
	       }
	    }	      
	 i=l;
	 j=l+l;
	 while ( j<=ir)
	       {
	       fseek ( fp, start+((j-1)*len*size), SEEK_SET);
	       fread (ra_j, size, len, fp);
	       
	       if ( j<ir)
	          {
		  fseek ( fp, start+(((j-1)+1)*len*size), SEEK_SET);
		  fread (ra_j_1, size, len, fp);
		  }

	       if ( j<ir && compare( ra_j, ra_j_1,first_comp_field,n_comp_fields, size )<0)
		   {
		   SWAPP(ra_j, ra_j_1, tp);
		   j++;
		   }

	       if (compare(rra, ra_j, first_comp_field,n_comp_fields, size)<0)
	               {
		       fseek ( fp, start+((i-1)*len*size), SEEK_SET);
		       fwrite(ra_j,size, len, fp);
		       i=j;
		       j<<= 1;
		       }
	       else
		       j=ir+1;
	       
	       }
	 fseek ( fp, start+((i-1)*len*size), SEEK_SET);
	 fwrite(rra,size, len, fp);	
	 }
     vfree (  rra);
     vfree ( rrb);

     vfree (  ra_j);
     vfree (  ra_j_1);
     return fp;
     }
void ** hsort_array ( void **ra,int n,int len, size_t size,int first_comp_field, int n_comp_fields,int (*compare)(const void *, const void*,int, int, size_t),void * (*copy)(void *,void*, size_t))
     {
     unsigned long i, ir, j, l;
     void *rra;
     int FE=1;
     
     if ( FE==1){ra--;}
     else
     {
     n--;
     }

     
     rra   =vcalloc ( len, size);
    
     
     if ( n<2)return ra;
     l=(n >>1)+1;
     ir=n;

     for (;;)
         {
	 if ( l>FE)
	    {
	    copy ( rra, ra[--l],len);
	    }
	 else
	    {	   
	    copy ( rra, ra[ir],len);
	    copy ( ra[ir], ra[FE], len);
	    if (--ir ==FE)
	       {
	       copy ( ra[FE],rra,len);
	       break;
	       }
	    }
	 i=l;
	 j=l+l;
	 while ( j<=ir)
	       {	       
	       if ( j<ir && compare( ra[j], ra[j+1],first_comp_field,n_comp_fields, size )<0)j++;
	       if (compare(rra, ra[j], first_comp_field,n_comp_fields, size)<0)
	               {copy(ra[i], ra[j],len);i=j;j<<= 1;}
	       else
		       j=ir+1;	       
	       }
	 copy( ra[i], rra,len);	 
	 }
     vfree (rra);
     ra+=FE;
     
     return ra;
     }
/*********************************************************************/
/*                                                                   */
/*                         CEDRIC BSEARCH                            */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
void * bsearch_file ( const void *key,int *p,int comp_first,int comp_len, FILE *fp,int len, int entry_len,size_t el_size, int (*compare)(const void *, const void*,int, int, size_t))
       {
       int upper, lower, c, i;
       static void *key2;
       long start;
       static long key2_size;


       start=ftell(fp);

       upper=-1;
       lower=len;
       if ( key2==NULL){key2=vcalloc (entry_len, el_size);key2_size=entry_len* el_size;}
       else if (key2_size<  (entry_len* el_size)){vfree(key2);key2=vcalloc (entry_len, el_size);key2_size=entry_len* el_size;} 

       while ((lower-upper)>1)
             {
	     i=(lower+upper) >> 1;

	     fseek ( fp,start+(i*el_size*entry_len), SEEK_SET); 
	     fread ( key2, el_size, entry_len,fp);	     
	     c=compare(key2,key, comp_first, comp_len,el_size);
	     
	     if      ( c==0){p[0]=i;return key2;}
	     else if ( c< 0)upper=i;
	     else if ( c> 0)lower=i;
             }
       return NULL;
       }

void * bsearch_array ( const void *key,int *p,int comp_first, int comp_len,void**list,int len, int entry_len,size_t el_size, int (*compare)(const void *, const void*,int, int, size_t))
       {
       int upper, lower, c, i;
       void *key2;
       
       upper=-1;
       lower=len;
       while ((lower-upper)>1)
             {
	     i=(lower+upper) >>1;
	     key2=list[i];	     
	     c=compare(key2,key, comp_first,comp_len,el_size);
	     
	     if      ( c==0){p[0]=i;return key2;}
	     else if ( c< 0)upper=i;
	     else if ( c> 0)lower=i;
             }
       return NULL;
       }	             
		     
/**********************************************************************/
/*                                                                    */
/*                         HSORT/BSEARCH WRAPPERS                             */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
void **search_in_list_file ( void *key, int *p,int comp_len,FILE *fp, int len, size_t size, int entry_len)  
      {
      static void **l;	
      

      if ( l==NULL)l=vcalloc ( 1, sizeof (int*));
	
      l[0]=bsearch_file (key,p,0,comp_len,fp,len,entry_len,size,hsort_cmp);  
      if (l[0]==NULL)return NULL;
      else return l;
      }
void **search_in_list_array ( void *key,int *p, int comp_len,void **L, int len, size_t size, int entry_len)  
      {
      static void **l;
 
      if ( l==NULL)l=vcalloc ( 1, sizeof (int*));	
      
      l[0]=bsearch_array (key,p,0,comp_len,L,len,entry_len,size,hsort_cmp);  
      if (l[0]==NULL)return NULL;
      else return l;
      }
void **hsort_list_array ( void **L, int len, size_t size, int entry_len, int first_comp_field, int n_comp_fields)  
       {
	 return hsort_array (L, len,entry_len, size,first_comp_field, n_comp_fields,hsort_cmp , hsort_cpy);
       }
FILE  *hsort_list_file ( FILE*fp  , int len, size_t size, int entry_len, int first_comp_field, int n_comp_fields)  
       {

       return hsort_file (fp, len,entry_len, size,first_comp_field, n_comp_fields,hsort_cmp , hsort_cpy);
       }

int hsort_cmp ( const void *a, const void *b, int first, int clen, size_t size)
       {
       int*ax;
       int*bx;
       int p;
       
       ax=(int*)a;
       bx=(int*)b;
       for ( p=first; p<clen+first; p++)
	   {
	   if ( ax[p]<bx[p])return -1;
	   else if ( ax[p]==bx[p]);
	   else return 1;
	   }
       return 0;
       }
void *hsort_cpy(void*to, void *from, size_t size)
       {
       
       int *ax;
       int *bx;
       int p;
       ax=(int*)to;
       bx=(int*)from;
       for (p=0; p<(int)size; p++)
	   ax[p]=bx[p];
       
       
       return to;
      
       }
       
       
void test_hsort_list_array()
      {
      int **array;
      int a;
      int n=100;

      array=declare_int(n, 3);
      for ( a=0; a<n; a++)array[a][0]=a;
      
      hsort_list_array( (void**)array,n, sizeof (int), 3, 0, 1);
      for ( a=0; a<n; a++)fprintf ( stderr, "\n%d %d", array[a][0],a);
      myexit(EXIT_FAILURE);
      }
      

/*********************************************************************/
/*                                                                   */
/*                         B_SEARCH_FILE FUNCTIONS                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/		 

/*********************************************************************/
/*                                                                   */
/*                         SORT/COMPARE/SEARCH FUNCTIONS             */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
static int sort_field;
int **search_in_list_int ( int *key, int k_len, int **list, int ne)
	{
	int **l;		
	sort_field=k_len;	
	l=bsearch (&key,list, ne, sizeof(int**),(int(*)(const void*,const void*))(cmp_list_int));  
	return l;
	}
void sort_float ( float **V,int N_F, int F, int left, int right)
	{
	sort_field=F;
	qsort ( V, right+1, sizeof(int**),(int(*)(const void*,const void*))(cmp_int));
	}
int cmp_float ( const float **a, const float **b)
	{
	if ( a[0][sort_field]< b[0][sort_field])return-1;
	else if ( a[0][sort_field]==b[0][sort_field])return 0;
	else return 1;
	}

void sort_int_1D ( int *L, int n)
	{
	int **array;
	int a;
	
	array=declare_int ( n, 1);
	for ( a=0; a< n; a++)
		array[a][0]=L[a];
	sort_int ( array, 1, 0, 0, n-1);
	for ( a=0; a< n; a++)
		L[a]=array[a][0];
	free_int ( array, n);
	}

char** sort_string_array (char **V, int n)     
{
  
  
  qsort ( V,n,sizeof (char*),(int(*)(const void*,const void*))(pstrcmp));
  return V;

 
}

int pstrcmp(char *p1, char *p2)
{
  return strcmp(*(char **)p1, *(char **)p2);
}
void sort_int ( int **V,int N_F, int F, int left, int right)
	{
	  if (!V)return;
	sort_field=F;
	qsort ( V, (right-left)+1, sizeof(int**),(int(*)(const void*,const void*))(cmp_int));
	}
void sort_list_int ( int **V,int N_F, int F, int left, int right)
        {
	if (!V)return;
	sort_field=F;
	qsort ( V, (right-left)+1, sizeof(int**),(int(*)(const void*,const void*))(cmp_list_int));
	}
static int *order;
void sort_list_int2 ( int **V,int *list,int N_F, int left, int right)
        {
	  // just like sort_int_list, but uses list to to order the comparison of the keys
	  if (!V)return;
	order=list;
	
	qsort ( V, (right-left)+1, sizeof(int**),(int(*)(const void*,const void*))(cmp_list_int2));
	}
int cmp_list_int2 (const int**a, const int**b)
	{
	  int p=0;;
	  int c;

	  

	  while ((c=order[p])!=-1)
	  {
	    if ( a[0][c]>b[0][c])return   1;
	    else if ( a[0][c]<b[0][c])return  -1;
	    p++;
	  }
	return 0;
	}
void sort_int_inv ( int **V,int N_F, int F, int left, int right)
	{
	int a,b;
	int **list;
	if (!V)return;
	sort_field=F;
	qsort ( V, (right-left)+1, sizeof(int**),(int(*)(const void*,const void*))(cmp_int));
	
	list=declare_int ((right-left)+1, N_F);
	for ( a=left; a< (right-left)+1; a++)
		{
		for ( b=0; b< N_F; b++)
			{
			list[a-left][b]=V[a][b];
			}
		}
	for ( a=left; a< (right-left)+1; a++)
		{
		for ( b=0; b< N_F; b++)
			V[a][b]=list[(right-left)-a][b];
		}
	free_int (list, -1);
	}
	
void sort_list_int_inv ( int **V,int N_F, int F, int left, int right)
	{
	int a,b;
	int **list;
	if (!V)return;
	sort_field=F;
	qsort ( V, (right-left)+1, sizeof(int**),(int(*)(const void*,const void*))(cmp_list_int));
	
	list=declare_int ((right-left)+1, N_F);	
	for ( a=left; a< (right-left)+1; a++)
		{
		for ( b=0; b< N_F; b++)
			{
			list[a-left][b]=V[a][b];
			}
		}
	for ( a=left; a< (right-left)+1; a++)
		{
		for ( b=0; b< N_F; b++)
			V[a][b]=list[(right-left)-a][b];
		}
	free_int (list, -1);
	}	
	


int cmp_int ( const int**a, const int**b)
	{
	if ( a[0][sort_field]< b[0][sort_field])return-1;
	else if ( a[0][sort_field]==b[0][sort_field])return 0;
	else return 1;
	}
int cmp_list_int (const int**a, const int**b)
	{
	int c;
	int undef=0;
	int ret;
	
	for ( c=0; c<=sort_field; c++)
		{
		if ( a[0][c]==UNDEFINED|| b[0][c]==UNDEFINED)ret=0;
		else if ( a[0][c]>b[0][c])return   1;
		else if ( a[0][c]<b[0][c])return  -1;
		}
	if (undef==sort_field)
		{
		  if (a[0][0]==b[0][0])return 0;
		}
	return 0;
	}


int name_is_in_list ( char *name, char **name_list, int n_name, int len)
	{
	int a;
	int pos=-1;
	/*Note: RETURNS THE Offset of the LAST Occurence of name in name_list*/
	
	if ( name_list==NULL || name ==NULL)return -1;
	

	for ( a=0; a< n_name; a++)
	  {
	    if ( name_list[a]==NULL);
	    else if ( len!=-1)
	      {
		if (strncmp ( name, name_list[a], len)==0)pos=a;	
	      }
	    else if ( strm ( name, name_list[a]))pos=a;
	  }
	return pos;
	}
char * check_list_for_dup ( char **list, int ne)
        {
	int a, b;
	
	for ( a=0; a< ne-1; a++)
	    for ( b=a+1; b< ne; b++)if (strm ( list[a], list[b]))return list[a];
	return NULL;
	}
FILE *get_number_list_in_file ( FILE *fp, int *list, int *n, int *max_len)
	{
	
	int c;
	
	while ( isspace((c=fgetc (fp))));
	ungetc(c, fp);
	while ( c!='\n')
		{
		while ( isspace((c=fgetc (fp))) && c!='\n');
		
		if ( c!='\n')
			{
			ungetc(c, fp);
			if ( n[0]>=max_len[0])
				list=vrealloc ( list, (n[0]+100)*sizeof (int));
				max_len[0]=(n[0]+100);
			
			fscanf ( fp, "%d",&list[n[0]++]);
			}
		}
	return fp;
	}
/*********************************************************************/
/*                                                                   */
/*                         QUANTILE                                  */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
int quantile (int argc, char *argv[])
{
  FILE *fp;
  int a,n,c, t;
  int **list; 
  char ***  string_list;
  char *name, s1[1000], s2[1000];


  if ( argc<2)
    {

      fprintf (stderr, "\nquantile <fname> <quant: 0.00-1.00> [<top | bottom>]");
      fprintf (stderr, "\nSplits your data in two according to the quantile");
      fprintf (stderr, "\nReturns the top quantile or the bottom quantile");
      fprintf (stderr, "\nData must be in <fname> with two fields/line: Field1=index, Field2=value\n");
      fprintf (stderr, "\n1 27\n2 46\n3 5\n...\n");
      fprintf (stderr, "\nValue can either be integer or float");
      
      
      myexit (EXIT_FAILURE);
    }
  
  if (strm (argv[1], "stdin"))
    {
      name=vtmpnam(NULL);
      fp=vfopen (name, "w");
      while ( (c=fgetc(stdin))!=EOF)
	{
	  fprintf ( fp, "%c", c);
	}
      vfclose (fp);
    }
  else
    name=argv[1];
  
  


  n=count_n_line_in_file (name);
  list=declare_int (n, 2);
  string_list=declare_arrayN(3,sizeof (char), n, 2, 10);
  
  fp=vfopen (name, "r");
  n=0;
  while ( (c=fgetc (fp))!=EOF)
    {
      ungetc(c,fp);
      fscanf ( fp, "%s %s\n", s1, s2);
      list[n][0]=(int)(atof(s1)*1000);
      list[n][1]=(int)(atof(s2)*1000);
      list[n][2]=n;
      
      sprintf (string_list[n][0],"%s",s1);
      sprintf (string_list[n][1],"%s",s2);

      
      n++;
    }
  sort_int_inv ( list,3, 1, 0, n-1);
  t=quantile_rank ( list,1,n, atof (argv[2]));
  if ( argc!=4 || (argc==4 && strm (argv[3], "bottom")))
    {
      for (a=t; a<n; a++)
	fprintf ( stdout, "%s %s\n", string_list[list[a][2]][0], string_list[list[a][2]][1]); 
    }
  else
    {
      for ( a=0; a<t; a++)
	fprintf ( stdout, "%s %s\n", string_list[list[a][2]][0], string_list[list[a][2]][1]);
    }
  
  fprintf (stderr, "\nQuantile %.2f T= %d Out of N= %d entries\n", atof (argv[2]), t, n), 
  free_int (list, -1);
  return n;

}
  
int quantile_rank (int **list,int field, int n, float p)
{
  int nr;
  
  
  if ( p==1) nr=0;
  else if ( p==0) nr=n;
  else
    {
      int a, b,j, *l;
      double g, q, np, i_part;
      l=vcalloc ( n, sizeof (int));
      for (a=n-1, b=0; b<n; a--, b++)
	l[b]=list[a][field];
            
      np=(double)n*(double)p;
      g=modf (np, &i_part);
      j=(int)i_part;
      j--;
      
      q=(float)l[j]+g*((float)l[j+1]-(float)l[j]);
      
      nr=0;
      while (nr<n && list[nr][field]>=q)nr++;
      vfree(l);
    }
  return nr;
}
/*********************************************************************/
/*                                                                   */
/*                         DUPLICATION                               */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
short* ga_memcpy_short ( short *array1, short *array2, int n)
	{
	int a;
	
	for ( a=0; a< n; a++)
		array2[a]=array1[a];
	
	return array2;
	}
int * ga_memcpy_int ( int *array1, int *array2, int n)
	{
	int a;
	
	for ( a=0; a< n; a++)
		array2[a]=array1[a];
	
	return array2;
	}			
			
float* ga_memcpy_float ( float *array1, float *array2, int n)
	{
	int a;
	
	for ( a=0; a< n; a++)
		array2[a]=array1[a];
	
	return array2;
	}
double*  ga_memcpy_double (double *array1, double*array2, int n)
	{
	int a;
	
	for ( a=0; a< n; a++)
		array2[a]=array1[a];
	
	return array2;
	}



/*recycle: get the bottom pointer on the top of the heap*/

void ** recycle (void **A, int l, int cycle)
{
  void **B;
  int a,b,c;
  B=vcalloc (l, sizeof (void*));
 
  for ( c=0; c< cycle; c++)
    {
      for ( a=1, b=0; a<l; a++, b++) B[b]=A[a];
      B[l-1]=A[0];
      for ( a=0; a<l; a++)A[a]=B[a];
    }
  vfree (B);
  return A;
} 

/* Old READ/WRITE ARRAY SIZE*/
/*
#define WRITE_SIZE(type,function)\
void function ( int x, type *array, int os)\
     {\
     int a,l;\
     char buf[SIZE_OF_INT+1];\
     array+=os*SIZE_OF_INT;\
     for ( a=0;a<SIZE_OF_INT; a++)array[a]=0;\
     sprintf ( buf, "%d", x);\
     l=strlen (buf);\
     array+=SIZE_OF_INT-l;\
     for (a=0; a<l; a++)array[a]=(type)buf[a];\
     }
WRITE_SIZE(short,write_size_short)
WRITE_SIZE(char,write_size_char)
WRITE_SIZE(int,write_size_int)
WRITE_SIZE(float,write_size_float)
WRITE_SIZE(double,write_size_double)

#define READ_ARRAY_SIZE(type, function)\
int function (type *array, int os)\
    {\
    int a, b;\
    char buf[SIZE_OF_INT+1];\
    a=b=0;\
    array+=os*SIZE_OF_INT;\
    while ( a!=SIZE_OF_INT && array[a]==0)a++;\
    while ( a!=SIZE_OF_INT)buf[b++]=(char)array[a++];\
    buf[b]='\0';\
    return atoi(buf);\
    }
READ_ARRAY_SIZE(short,read_size_short)
READ_ARRAY_SIZE(char,read_size_char)
READ_ARRAY_SIZE(int,read_size_int)
READ_ARRAY_SIZE(float,read_size_float)
READ_ARRAY_SIZE(double,read_size_double)
*/
/*********************************************************************/
/*                                                                   */
/*                          DUPLICATION                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#define SET_NUMBERS(type,function)\
type * function(type *list,int n, ...)\
     {\
     type *buf;\
     int  *index,i;\
     va_list ap;\
     int max;\
\
     va_start(ap, n);\
     buf=vcalloc(n, sizeof(type));\
     index=vcalloc(n, sizeof(int));\
     max=0;\
     for ( i=0; i< n; i++)\
         {\
	 buf[i]  =va_arg(ap,type);\
	 index[i]=va_arg(ap, int);\
	 if (index[i]>max)max=index[i];\
	 }\
     va_end(ap);\
     if (list==NULL)list=vcalloc ( max+1, sizeof (type));\
     for ( i=0; i<n; i++)list[index[i]]=buf[i];\
     vfree(buf);\
     vfree(index);\
     return list;\
     }
     /*SET_NUMBERS(short ,set_short)*/
     /*SET_NUMBERS(char  ,set_char)*/
SET_NUMBERS(int   ,set_int)
     /*SET_NUMBERS(float ,set_float)*/
SET_NUMBERS(double,set_double)

short ** duplicate_short ( short **array , int len, int field)
    {
    return copy_short (array ,declare_short ( len, field),  len, field);
    }
int ** duplicate_int ( int **array , int len, int field)
    {
    return copy_int (array ,declare_int ( len, field),  len, field);
    }
char ** duplicate_char ( char **array , int len, int field)
    {
    return copy_char (array ,declare_char ( len, field),  len, field);
    }
char * duplicate_string ( char *string)
    {
      int l;
      char *buf=NULL;
      
      l=strlen (string);
      
      if ( !l);
      else
	{
	  buf=vcalloc ( l+1, sizeof(char));
	  sprintf ( buf, "%s", string);
	}
      return buf;
    }
float ** duplicate_float ( float **array , int len, int field)
    {
    return copy_float (array ,declare_float ( len, field),  len, field);
    }
double ** duplicate_double ( double **array , int len, int field)
    {
    return copy_double (array ,declare_double ( len, field),  len, field);
    }



/*********************************************************************/
/*                                                                   */
/*                           COPY OF 2D ARRAY                        */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
short ** copy_short( short **array1, short  **array2, int len, int number_field)
    {
    int a;
    
    if ( len==-1)len=read_array_size (array1,sizeof (short*));
    if ( number_field==-1)number_field=read_array_size (array1[0],sizeof (short));
    if ( array2)free_short ( array2, -1);
    array2=declare_short ( len, number_field);

    for ( a=0; a< len; a++)
	ga_memcpy_short( array1[a],array2[a],number_field);
    
    return array2;
    }
char ** copy_char ( char **array1, char **array2, int len, int number_field)
    {
    int a;
    
    if ( array1==NULL)return NULL;
    if ( len==-1)len=read_size_char (array1,sizeof (char*));
    if ( number_field==-1)
	{
	number_field=read_size_char (array1[0],sizeof(char));
	for ( a=0; a< len; a++)
	    number_field=MAX(number_field, strlen ( array1[a]))+1;
	}
    
    if ( array2)free_char (array2, -1);
    array2=declare_char(len, number_field);
    
    for ( a=0; a< len; a++)
      sprintf ( array2[a], "%s", array1[a]);

    return array2;
    } 
int ** copy_int ( int **array1, int **array2, int len, int number_field)
    {
    int a;
    
    if ( array1==NULL) return NULL;
    if ( len==-1)len=read_size_int (array1, sizeof (int*));
    if ( number_field==-1)number_field=read_size_int (array1[0],sizeof (int));
    
    
    
    if (array2)free_int (array2, -1);
    array2=declare_int ( len, number_field);

    for ( a=0; a< len; a++)
	ga_memcpy_int( array1[a],array2[a],number_field);
    
    return array2;
    }

float ** copy_float ( float **array1, float **array2, int len, int number_field)
    {
    int a;
    
    if ( array1==NULL) return NULL;
    if ( len==-1)len=read_size_float (array1,sizeof (float*));
    if ( number_field==-1)number_field=read_size_float (array1[0],sizeof (float));

    if ( array2)free_float (array2, -1);
    array2=declare_float ( len, number_field);
    
    for ( a=0; a< len; a++)
	ga_memcpy_float( array1[a],array2[a],number_field);
    return array2;
    }
double ** copy_double (double **array1, double **array2, int len, int number_field)
    {
    int a;

    if ( array1==NULL) return NULL;
    if ( len==-1)len=read_size_double (array1,sizeof (double*));
    if ( number_field==-1)number_field=read_size_double (array1[0], sizeof (double));
    if ( array2)free_double (array2, -1);
    array2=declare_double ( len, number_field);

    for ( a=0; a< len; a++)
	ga_memcpy_double( array1[a],array2[a],number_field);
    return array2;
    }
/*********************************************************************/
/*                                                                   */
/*                        CONCATENATION                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/





Alignment ** cat_aln_list ( Alignment **list_to_cat,int first, int end, Alignment **rec_list)
    {
    int rec_list_start;
    int a, b;

    if ( list_to_cat==NULL)return rec_list;
    else
       {
       rec_list_start=(rec_list[-1])->nseq;       
       rec_list=realloc_aln_array ( rec_list, end-first);      
       for ( a=first, b=rec_list_start; a<end; a++, b++)copy_aln (list_to_cat[a], rec_list[b]);      
       free_aln_array ( list_to_cat);      
       return rec_list;
       }
    }
	   
/*********************************************************************/
/*                                                                   */
/*                         NUMBER ARRAY ANALYSE                      */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

int output_array_int (char* fname, int  **array)
     {
     int a, b;
     int len,nf;
     FILE *fp;

     fp=vfopen (fname, "w");
     len=read_array_size_new (array);
     fprintf (fp, "%d\n",len);
     for ( a=0; a<len; a++)
       {
	 nf=read_array_size_new(array[a]);
	 fprintf (fp, "%d ",nf);
	 for (b=0; b<nf; b++)fprintf (fp, "%5d ", array[a][b]);
	 fprintf (fp, "\n");
       }
     fprintf ( fp, "\n");
     vfclose (fp);
     return EXIT_SUCCESS;
     }

int **input_array_int (char *fname)
{
  FILE *fp;
  int **array;
  int len,len2,a,b;

  fp=vfopen (fname, "r");
  fscanf (fp, "%d\n", &len);
  array=vcalloc (len, sizeof (int*));
  for (a=0; a<len; a++)
    {
      fscanf(fp, "%d ", &len2);
      array[a]=vcalloc (len2, sizeof (int));
      for (b=0; b<len2; b++)
	fscanf(fp, "%d ", &array[a][b]);
      fscanf (fp, "\n");
    }
  vfclose (fp);
  return array;
}


#define RETURN_MAX_COOR(type,wf,rf,function,comparison,undef)\
type function ( type ** array, int len_array, int field, int *coor)\
    {\
    type max;\
    int a;\
\
    if (array==NULL || len_array==0)return 0;\
    else\
        {\
        len_array=rf(array,sizeof (type*));\
        max=array[0][field];\
        coor[0]=0;\
        for ( a=1; a< len_array; a++)\
	      if ( max==undef)max=array[a][field];\
	      else if ( array[a][field]!=undef)\
		   if (array[a][field] comparison max)\
		       {max=array[a][field];\
                        coor[0]=a;\
	                }\
	 }\
    return max;\
    }

RETURN_MAX_COOR(short,write_size_short,read_size_short,return_max_coor_short,>, UNDEFINED_SHORT)
RETURN_MAX_COOR(char,write_size_char,read_size_char,return_max_coor_char,>, UNDEFINED_CHAR)
RETURN_MAX_COOR(int,write_size_int,read_size_int,return_max_coor_int,>, UNDEFINED_INT)
RETURN_MAX_COOR(float,write_size_float,read_size_float,return_max_coor_float,>, UNDEFINED_FLOAT)
RETURN_MAX_COOR(double,write_size_double,read_size_double,return_max_coor_double,>, UNDEFINED_DOUBLE)
RETURN_MAX_COOR(short,write_size_short,read_size_short,return_min_coor_short,<, UNDEFINED_SHORT)
RETURN_MAX_COOR(char,write_size_char,read_size_char,return_min_coor_char,<, UNDEFINED_CHAR)
RETURN_MAX_COOR(int,write_size_int,read_size_int,return_min_coor_int,<, UNDEFINED_INT)
RETURN_MAX_COOR(float,write_size_float,read_size_float,return_min_coor_float,<, UNDEFINED_FLOAT)
RETURN_MAX_COOR(double,write_size_double,read_size_double,return_min_coor_double,<, UNDEFINED_DOUBLE)
#define RETURN_MAX(type,wf,rf,function,comparison,undef)\
type function ( type ** array, int len_array, int field)\
    {\
    type max;\
    int a;\
\
    if (array==NULL || len_array==0)return 0;\
    else\
        {\
        if (len_array==-1)len_array=rf(array,sizeof (type*));\
        max=array[0][field];\
        for ( a=1; a< len_array; a++)\
            if ( max==undef)max=array[a][field];\
	    else if ( array[a][field]!=undef)max=( array[a][field] comparison max)?array[a][field]:max;\
        }\
    return (max==undef)?0:max;\
    }

RETURN_MAX(short,write_size_short,read_size_short,return_max_short,>,UNDEFINED_SHORT)
RETURN_MAX(char,write_size_char,read_size_char,return_max_char,>,UNDEFINED_CHAR)
RETURN_MAX(int,write_size_int,read_size_int,return_max_int,>,UNDEFINED_INT)
RETURN_MAX(float,write_size_float,read_size_float,return_max_float,>,UNDEFINED_FLOAT)
RETURN_MAX(double,write_size_double,read_size_double,return_max_double,>,UNDEFINED_DOUBLE)
RETURN_MAX(short,write_size_short,read_size_short,return_min_short,<,UNDEFINED_SHORT)
RETURN_MAX(char,write_size_char,read_size_char,return_min_char,<,UNDEFINED_CHAR)
RETURN_MAX(int,write_size_int,read_size_int,return_min_int,<,UNDEFINED_INT)
RETURN_MAX(float,write_size_float,read_size_float,return_min_float,<,UNDEFINED_FLOAT)
RETURN_MAX(double,write_size_double,read_size_double,return_min_double,<,UNDEFINED_DOUBLE)



#define RETURN_2DMAX(type,wf,rf,function,comparison,undef)\
type function ( type ** array, int start, int len_array, int first_field, int number_field)\
    {\
    type max;\
    int a,b;\
    if (array==NULL || len_array==0 || first_field<0 || number_field==0)return 0;\
    else\
         {max=array[start][first_field];\
          for ( a=start; a< start+len_array; a++)\
	      for (b=first_field; b< first_field+number_field; b++)\
	             if (array[a][b]!=undef)max=( array[a][b] comparison max)?array[a][b]:max;\
         }\
    return max;\
    }
RETURN_2DMAX(short,write_size_short,read_size_short,return_2Dmax_short,>, UNDEFINED_SHORT)
RETURN_2DMAX(char,write_size_char,read_size_char,return_2Dmax_char,>,UNDEFINED_CHAR)
RETURN_2DMAX(int,write_size_int,read_size_int,return_2Dmax_int,>,UNDEFINED_INT)
RETURN_2DMAX(float,write_size_float,read_size_float,return_2Dmax_float,>,UNDEFINED_FLOAT)
RETURN_2DMAX(double,write_size_double,read_size_double,return_2Dmax_double,>,UNDEFINED_DOUBLE)
RETURN_2DMAX(short,write_size_short,read_size_short,return_2Dmin_short,<,UNDEFINED_SHORT)
RETURN_2DMAX(char,write_size_char,read_size_char,return_2Dmin_char,<,UNDEFINED_CHAR)
RETURN_2DMAX(int,write_size_int,read_size_int,return_2Dmin_int,<,UNDEFINED_INT)
RETURN_2DMAX(float,write_size_float,read_size_float,return_2Dmin_float,<,UNDEFINED_FLOAT)
RETURN_2DMAX(double,write_size_double,read_size_double,return_2Dmin_double,<,UNDEFINED_DOUBLE)

#define RETURN_2DMAX_COOR(type,wf,rf,function,compare,undef)\
type function ( type **array, int start1 , int end1, int start2, int end2,int *i, int *j)\
    {\
    int a, b;\
    double max=undef;\
    if ( start1==-1)start1=0;\
    if ( start2==-1)start2=0;\
    if ( end1==-1)end1=rf(array,sizeof (type*));\
    if ( end2==-1)end2=rf(array[0],sizeof (type));\
    if ( array==NULL || (end1-start1)==0 || (end1-start1)>rf ( array,sizeof (type*)) || (end2-start2)==0)\
        {\
	return 0;\
        i[0]=0;\
        j[0]=0;\
        }\
    i[0]=0;\
    j[0]=0;\
    for ( a=start1; a<end1; a++)\
	for ( b=start2; b<end2; b++)\
	    {\
            if ( max==undef && array[a][b]!=undef)max=array[a][b];\
	    else if ( array[a][b]!=undef && (array[a][b] compare max))\
	       {\
	       max=array[a][b];\
	       i[0]=a;\
	       j[0]=b;\
	       }\
	    }\
    return (type)max;\
    }
RETURN_2DMAX_COOR(short,write_size_short,read_size_short,return_2Dmax_coor_short,>,UNDEFINED_SHORT)
RETURN_2DMAX_COOR(char,write_size_char,read_size_char,return_2Dmax_coor_char,>,UNDEFINED_CHAR)
RETURN_2DMAX_COOR(int,write_size_int,read_size_int,return_2Dmax_coor_int,>,UNDEFINED_INT)
RETURN_2DMAX_COOR(float,write_size_float,read_size_float,return_2Dmax_coor_float,>,UNDEFINED_FLOAT)
RETURN_2DMAX_COOR(double,write_size_double,read_size_double,return_2Dmax_coor_double,>,UNDEFINED_DOUBLE)
RETURN_2DMAX_COOR(short,write_size_short,read_size_short,return_2Dmin_coor_short,<,UNDEFINED_SHORT)
RETURN_2DMAX_COOR(char,write_size_char,read_size_char,return_2Dmin_coor_char,<,UNDEFINED_CHAR)
RETURN_2DMAX_COOR(int,write_size_int,read_size_int,return_2Dmin_coor_int,<,UNDEFINED_INT)
RETURN_2DMAX_COOR(float,write_size_float,read_size_float,return_2Dmin_coor_float,<,UNDEFINED_FLOAT)
RETURN_2DMAX_COOR(double,write_size_double,read_size_double,return_2Dmin_coor_double,<,UNDEFINED_DOUBLE)

#define RETURN_WMEAN(type,wf,rf,function,sum_function,undef)\
double function ( type **array, int len, int wfield,int sfield)\
    {\
    double b;\
    int a, c;\
    if ( len==0 ||array==NULL || len>rf ( array,sizeof (type*)))return 0;\
    else\
         {\
         if ( len==-1)len=rf(array,sizeof (type*));\
         for ( b=0, c=0,a=0; a< len; a++)\
             {\
	     if (array[a][sfield]!=undef && array[a][wfield]!=undef )\
	        {\
		b+=array[a][sfield];\
		c+=array[a][wfield];\
		}\
             }\
         }\
    return (c==0)?0:(b/c);\
    }
RETURN_WMEAN(short,write_size_short,read_size_short,return_wmean_short, return_sum_short,UNDEFINED_SHORT)
RETURN_WMEAN(char,write_size_char,read_size_char, return_wmean_char,return_sum_char,UNDEFINED_CHAR)
RETURN_WMEAN(int,write_size_int,read_size_int,return_wmean_int,return_sum_int,UNDEFINED_INT)
RETURN_WMEAN(float,write_size_float,read_size_float,return_wmean_float,return_sum_float,UNDEFINED_FLOAT)
RETURN_WMEAN(double,write_size_double,read_size_double,return_wmean_double,return_sum_double,UNDEFINED_DOUBLE)

		     
#define RETURN_MEAN(type,wf,rf,function,sum_function,undef)\
double function ( type **array, int len, int field)\
    {\
    double b;\
    int a, c;\
    if ( len==0 ||array==NULL || len>rf ( array,sizeof(type*)))return 0;\
    else\
         {\
         for ( b=0, c=0,a=0; a< len; a++)\
             {\
	     if (array[a][field]!=undef)\
	        {\
		b+=array[a][field];\
		c++;\
		}\
             }\
         }\
    return (c==0)?0:(b/c);\
    }
RETURN_MEAN(short,write_size_short,read_size_short,return_mean_short, return_sum_short,UNDEFINED_SHORT)
RETURN_MEAN(char,write_size_char,read_size_char, return_mean_char,return_sum_char,UNDEFINED_CHAR)
RETURN_MEAN(int,write_size_int,read_size_int,return_mean_int,return_sum_int,UNDEFINED_INT)
RETURN_MEAN(float,write_size_float,read_size_float,return_mean_float,return_sum_float,UNDEFINED_FLOAT)
RETURN_MEAN(double,write_size_double,read_size_double,return_mean_double,return_sum_double,UNDEFINED_DOUBLE)

#define RETURN_SUM(type,wf,rf,function,undef)\
type function(type **array, int len, int field)\
{\
 int a;\
 type b=0;\
 if ( len==0 ||array==NULL)return 0;\
 else\
     {\
     if ( len==-1)len=rf ( array,sizeof (type*));\
     for ( a=0; a< len; a++)\
          if ( array[a][field]!=undef)b+=array[a][field];\
     }\
  return b;\
  }
RETURN_SUM(short,write_size_short,read_size_short, return_sum_short,UNDEFINED_SHORT)
RETURN_SUM(char,write_size_char,read_size_char,return_sum_char,UNDEFINED_CHAR)
RETURN_SUM(int,write_size_int,read_size_int,return_sum_int,UNDEFINED_INT)
RETURN_SUM(float,write_size_float,read_size_float,return_sum_float,UNDEFINED_FLOAT)
RETURN_SUM(double,write_size_double,read_size_double,return_sum_double,UNDEFINED_DOUBLE)    

#define RETURN_SD(type,wf,rf,function,undef)\
  type function ( type **array, int len, int field,type mean)	\
    {\
    int a;\
    double c=0;\
    if ( len==0 ||array==NULL || len>rf ( array,sizeof(type*)))return 0;\
    else\
        {\
        for ( a=0; a< len; a++)\
    	    {\
	    if ((array[a][field]!=undef) && (mean-array[a][field])!=0)\
             c+=((double)mean-array[a][field])*((double)mean-array[a][field]);\
            }\
        c=sqrt(c)/(double)len;\
	return (type)MAX(c,1);\
	}\
    }
RETURN_SD(short,write_size_short,read_size_short, return_sd_short,UNDEFINED_SHORT)
RETURN_SD(char,write_size_char,read_size_char,return_sd_char,UNDEFINED_CHAR)
RETURN_SD(int,write_size_int,read_size_int,return_sd_int,UNDEFINED_INT)
RETURN_SD(float,write_size_float,read_size_float,return_sd_float,UNDEFINED_FLOAT)
RETURN_SD(double,write_size_double,read_size_double,return_sd_double,UNDEFINED_DOUBLE) 
double return_z_score( double x,double sum, double sum2, double n)
    {
    double sd;
    double avg;
    double z;
    
   
    sd=(n==0)?0:sqrt(sum2*n -sum*sum)/n;
    avg=(n==0)?0:(sum/n);
    z=(sd==0)?0:(x-avg)/sd;
    return z;
    }

double* return_r (double **list, int n)
{
   double Sy, Sx, Sxy, Sx2, Sy2,r_up, r_low, x, y;
   double *r;
   int a;
   
   r=vcalloc ( 3, sizeof (double));
   Sy=Sx=Sxy=Sx2=Sy2=0;
   
   for ( a=0; a<n; a++)
     {
       x=list[a][0];
       y=list[a][1];
       Sy+=y;
       Sx+=x;
       Sy2+=y*y;
       Sx2+=x*x;
       Sxy+=x*y;
     }
   r_up=n*Sxy-(Sx*Sy);
   r_low=(n*Sx2-(Sx*Sx))*(n*Sy2-(Sy*Sy));
   r_low=sqrt(r_low);
   r[0]=(r_low==0)?0:r_up/r_low;
   
   x=Sx/(double)n;
   y=Sy/(double)n;
   r_up=Sxy-(n*x*y);
   r_up*=r_up;
   r_low=(Sx2-n*x*x)*(Sy2-n*y*y);
   r[1]=(r_low==0)?0:r_up/r_low;
   r[2]=n;
   return r;
  }

#define INVERT_LIST(type,wf,rf,function,swap_function)\
type* function (type *list, int len)\
    {\
    int a, b;\
    for ( a=0, b=len-1; a<b; a++, b--)swap_function ( &list[a], &list[b], 1);\
    return list;\
    }
INVERT_LIST(short,write_size_short,read_size_short, invert_list_short,swap_short)
INVERT_LIST(char,write_size_char,read_size_char,invert_list_char,swap_char)
INVERT_LIST(int,write_size_int,read_size_int,invert_list_int,swap_int)
INVERT_LIST(float,write_size_float,read_size_float,invert_list_float,swap_float)
INVERT_LIST(double,write_size_double,read_size_double,invert_list_double,swap_double) 

#define SWAP_FUNCTION(type,wf,rf,function)\
void function(type *a, type *b, int n)\
    {\
    type t;\
    int c;\
    for ( c=0;c<n;c++)\
	{t=a[c];\
	 a[c]=b[c];\
	 b[c]=t;\
	}\
    }
SWAP_FUNCTION(short,write_size_short,read_size_short,swap_short)
SWAP_FUNCTION(char,write_size_char,read_size_char,swap_char)
SWAP_FUNCTION(int,write_size_int,read_size_int,swap_int)
SWAP_FUNCTION(float,write_size_float,read_size_float,swap_float)
SWAP_FUNCTION(double,write_size_double,read_size_double,swap_double) 

#define RETURN_MAX_HORIZ(type,wf,rf,function,comparison,undef)\
type function  (type ** array, int len_array, int field)\
    {\
    type max;\
    int a;\
    if ( len_array==0)return 0;\
    else\
        {\
	max=array[field][0];\
        for ( a=1; a< len_array; a++)\
	    if ( array[field][a]!=undef) max=( array[field][a] comparison max)?array[field][a]:max;\
        return (int)max;\
        }\
    }
RETURN_MAX_HORIZ(short,write_size_short,read_size_short,return_max_short_hor,>,UNDEFINED_SHORT)
RETURN_MAX_HORIZ(char,write_size_char,read_size_char,return_max_char_hor,>,UNDEFINED_CHAR)
RETURN_MAX_HORIZ(int,write_size_int,read_size_int,return_max_int_hor,>,UNDEFINED_INT)
RETURN_MAX_HORIZ(float,write_size_float,read_size_float,return_max_float_hor,>,UNDEFINED_FLOAT)
RETURN_MAX_HORIZ(double,write_size_double,read_size_double,return_max_double_hor,>,UNDEFINED_DOUBLE) 

RETURN_MAX_HORIZ(short,write_size_short,read_size_short,return_min_short_hor,<,UNDEFINED_SHORT)
RETURN_MAX_HORIZ(char,write_size_char,read_size_char,return_min_char_hor,<,UNDEFINED_CHAR)
RETURN_MAX_HORIZ(int,write_size_int,read_size_int,return_min_int_hor,<,UNDEFINED_INT)
RETURN_MAX_HORIZ(float,write_size_float,read_size_float,return_min_float_hor,<,UNDEFINED_FLOAT)
RETURN_MAX_HORIZ(double,write_size_double,read_size_double,return_min_double_hor,<,UNDEFINED_DOUBLE) 

  

#define BEST_OF_MANY(type,wf,rf,function,undef)\
type function (int n, ...)\
	{\
	va_list ap;\
	int *fop,a;\
	type v, best;\
	int maximise;\
	/*first Arg: number of values\
	  2nd   Arg: maximise(1)/minimise(0)\
	  3rd   Arg: *int contains the indice of the best value\
	  ...   Arg: n type values\
	*/\
	va_start (ap, n);\
	maximise=va_arg (ap, int);\
	fop=va_arg (ap, int*);\
	best=va_arg (ap, type);\
	fop[0]=0;\
	for ( a=1; a<n; a++)\
		{\
		v=va_arg (ap, type);\
		if (best==undef)\
			{\
			best=v;\
			fop[0]=a;\
			}\
		if ( best==undef || v==undef);\
		else if ( maximise==1 && v>best)\
			{\
			fop[0]=a;\
			best=v;\
			}\
		else if ( maximise==0 && v<best)\
			{\
			fop[0]=a;\
			best=v;\
			}\
		}\
	va_end (ap);\
	return best;\
	}
     /*BEST_OF_MANY(short,write_size_short,read_size_short, best_short,UNDEFINED_SHORT)*/
     /*BEST_OF_MANY(char,write_size_char,read_size_char,best_char,UNDEFINED_CHAR)*/
BEST_OF_MANY(int,write_size_int,read_size_int,best_int,UNDEFINED_INT)
     /*BEST_OF_MANY(float,write_size_float,read_size_float,best_float,UNDEFINED_FLOAT)*/
BEST_OF_MANY(double,write_size_double,read_size_double,best_double,UNDEFINED_DOUBLE)
#define IS_DEFINED(type,function,undef)\
int function(int n, ...)\
     {\
     int i;\
     va_list ap;\
\
     va_start(ap, n);\
     for ( i=0; i< n; i++)\
         {\
	 if(va_arg(ap,type)==undef)\
	      {\
		va_end(ap);\
		return 0;\
	      }\
	 }\
     va_end(ap);\
     return 1;\
     }	
     /*IS_DEFINED(short,is_defined_short,UNDEFINED_SHORT)*/
     /*IS_DEFINED(char,is_defined_char,  UNDEFINED_CHAR)*/
IS_DEFINED(int,is_defined_int,   UNDEFINED_INT)
     /*IS_DEFINED(float,is_defined_float, UNDEFINED_FLOAT)*/
IS_DEFINED(double,is_defined_double,UNDEFINED_DOUBLE)
  
  int return_maxlen ( char ** array, int number)
    {
    int a;
    int max=0;
    for ( a=0; a< number; a++)
	max=( strlen ( array[a])>max)?strlen ( array[a]):max;

    return max;
    }


int return_minlen ( char ** array, int number)
    {
    int a;
    int min;

    min=strlen( array[0]);
    for ( a=1; a< number; a++)
	min=( strlen ( array[a])>min)?strlen ( array[a]):min;

    return min;
    }
 


float return_mean_diff_float ( float **array, int len, int field,float mean)
    {
    int a;
    float b=0;
    
    for ( a=0; a< len; a++)
    	{
    	if ( (mean-array[a][field])!=0) 
	 	b+=sqrt((double)((float) ( mean-array[a][field])*(float)(mean-array[a][field])));
	}
	
    return ((float)b/(float)len);
    }



void inverse_int ( int**array, int len, int field, int max, int min)
    {
    int a;
    for ( a=0; a< len; a++)
	array[a][field]=max-array[a][field]+min;
    }
void inverse_float ( float**array, int len, int field, int max, int min)
    {
    int a;
    for ( a=0; a< len; a++)
	array[a][field]=max-array[a][field]+min;
    }
void inverse_2D_float ( float **array, int start, int len, int start_field, int number_field, float max,float min)
    {
    int a, b;
    for ( a=0; a< start+len; a++)
	for ( b=start_field; b< start_field+ number_field; b++)
	    array[a][b]=max-array[a][b]+min;
    }

int max_int (int*i, ...)
{ 
  va_list ap;					\
  int index, best_value=0, value;
  int a=0;
 //  expects n values : n, &index, i1, v1, i2, v2...., -1
     va_start(ap, i);
     while ((index=va_arg(ap,int))!=-1)
       {
	 value=va_arg(ap, int);
	 if ( a==0 || value>best_value)
	   {
	     i[0]=index;
	     best_value=value;
	     a=1;
	   }
       }
     va_end (ap);
     return best_value;
}
  
/*********************************************************************/
/*                                                                   */
/*                         SHELL INTERFACES                          */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
char* getenv4debug (const char * val)
{
  /*efficient mean of getting an environment variable: checks only if one DEBUG is on*/
  static int check;
  
  if ( !check)
    {
      
      if (getenv       ("DEBUG_BLAST"))check=1;      
      else if ( getenv ("DEBUG_TREE_COMPARE"))check=1;
      else if ( getenv ("DEBUG_MALN"))check=1;
      else if ( getenv ("DEBUG_EXTRACT_FROM_PDB"))check=1;
      else if ( getenv ("DEBUG_LIBRARY"))check=1;
      else if ( getenv ("DEBUG_FUGUE"))check=1;
      
      else if ( getenv ("DEBUG_REFORMAT"))check=1;
      else if ( getenv ("DEBUG_RECONCILIATION"))check=1;
      else if ( getenv ("DEBUG_TMP_FILE"))check=1;
      else if ( getenv ("DEBUG_TREE"))check=1;
      
     else if ( getenv ("DEBUG_SEQ_REFORMAT") && strm (PROGRAM, "SEQ_REFORMAT"))check=2;
      else if ( getenv ("DEBUG_TCOFFEE") && strm (PROGRAM, "T-COFFEE"))check=2;
      else check=-1;      
    }

  if ( check>0 && strm ( val, "DEBUG_TMP_FILE"))
    {
      return "1";
    }
  
  else if ( check==1)
    {
      return getenv (val);
    }
  else if ( check==2)
    {
      return "1";
    }
  else
    return NULL;
}

int   atoigetenv (const char*var)
{
  char *v;
  if (!var) return 0;
  else if (!(v=getenv(var)))return 0;
  else if ( is_number(v))return atoi(v);
  else return 1;
}
char* get_env_variable ( const char *var, int mode)
        {
	    /*mode 0: return NULL if variable not set*/
	    /*mode 1: crash if variable not set*/
	    if ( !getenv (var))
	       {
		   if (mode==NO_REPORT)return NULL;
		   else if ( mode ==IS_NOT_FATAL)
		     {
		       myexit(fprintf_error ( stderr, "\nYou must set the variable %s [FATAL]\n", var));
		       return NULL;
		      }
		   else
		      {
			  myexit(fprintf_error ( stderr, "\nYou must set the variable %s [FATAL]\n", var));
	 		  myexit (EXIT_FAILURE);
			  return NULL;
		      }
	       }
	    else return getenv (var);
	}
		
void get_pwd ( char *name)
	{
	char *string;
	FILE *fp;
	 

	string=vtmpnam(NULL);
	printf_system_direct ("pwd > %s", string);
	
	fp=vfopen ( string, "r");
	fscanf ( fp, "%s",name);
	vfclose (fp);
	printf_system_direct ("rm %s", string);
	}
int pg_is_installed ( char *pg)
        {
	char *fname;
	FILE *fp;
	int r=0;

	return 1;

	fname= vtmpnam(NULL);
	
	printf_system_direct("which %s > %s", pg, fname);
	
	if ((fp=find_token_in_file ( fname, NULL, "Command"))){r=1;vfclose(fp);}
	 

	return r;
	
	}

	    
/*********************************************************************/
/*                                                                   */
/*                           MISC                                    */  
/*                                                                   */
/*********************************************************************/
char *num2plot (int value, int max, int line_len)
        {
	       int   len;
	       int   value_len;
	       char *buf;
	static char *string;

	if ( string==NULL)string=vcalloc (1000, sizeof(char));
 
	if ( line_len==-1)len=30;
	else len=line_len;
	
	value_len=((float)value/(float)max)*(float)len;
	if ( value==0)
	    sprintf ( string, "|");
	else
	    {
	    buf=generate_string(value_len, '*');
	    sprintf ( string,"%s", buf);
	    vfree(buf);
	    }
	return string;
	}
	
int   perl_strstr ( char *string, char *pattern)
{
  char *tmp;
  FILE *fp;
  int r;
  
  char *string2;

  if (!string)  return 0;
  if (!pattern) return 0;
  
  
  
  string2=vcalloc ( strlen (string)+1, sizeof (char));
  sprintf ( string2,"%s", string);
  string2=substitute (string2, "(", " ");
  string2=substitute (string2, ")", " ");
  string2=substitute (string2, "'", " ");
  tmp=vtmpnam(NULL);
  printf_system_direct("perl -e '$s=\"%s\";$x=($s=~/%s/);$x=($x==1)?1:0;print $x;'>%s", string2, pattern,tmp);
  
  if (check_file_exists(tmp))
    {
      fp=vfopen (tmp, "r");
      fscanf (fp, "%d", &r);
      vfclose (fp);
    }
  else
    {
      fprintf ( stderr, "COM: %s\n", string);
      r=0;
    }
  vfree (string2);
  return r;
}
float grep_function ( char *pattern, char *file)
	{

	int a, b, l;
	char buf1[100];
	char buf2[100];
	FILE*fp;
	char *s;
	float f;
	
	

	s=vtmpnam(NULL);
	
	printf_system_direct("grep %s %s > %s",pattern,file, s);
	
	if ((fp=vfopen (s, "r"))==NULL )return 0;
	else
		{
		  char command[1000];
		  fgets ( command, 900, fp);
		  l=strlen ( command);
		  while ( !isdigit (command[l]))l--;
		  a=0;
		  while ( isdigit (command[l]) || command[l]=='.')
		    {
		      buf1[a++]=command[l];
		      l--;
		    }
		  buf1[a]='\0';
		  l=strlen (buf1);
		  for ( a=0, b=l-1;a< l; a++, b--)
		    buf2[b]=buf1[a];
		  buf2[l]='\0';
		  
		  sscanf ( buf2, "%f", &f);
		  printf_system_direct("rm %s", s);
		  
		  vfclose (fp);
		  return f;
		}
	}    

void crash_if ( int val, char *s)
    {
    if ( val==0)crash(s);
    }
void crash ( char *s)
	{
	int *a;

	

	fprintf ( stderr, "%s",s);
	a=vcalloc ( 10, sizeof (int));
	a[20]=1;
	error_exit();
	}

static int *local_table;
int ** make_recursive_combination_table ( int tot_n_param, int *n_param, int *nc, int**table, int field)
    {
    int a, b, c;
    
    /* makes a table of all possible combinations*/

    if ( tot_n_param==0)
	{
	    nc[0]=1;
	    fprintf ( stderr, "\nNULL RETURNED");
	    return NULL;
	}
    if (table==NULL)
        {
        if ( local_table!=NULL)vfree (local_table);
	local_table=vcalloc ( tot_n_param, sizeof (int));
	field=0;
	for ( a=0; a< tot_n_param; a++)local_table[a]=-1;
	for ( a=0; a< tot_n_param; a++)nc[0]=nc[0]*n_param[a];
	

	table=declare_int ( nc[0],tot_n_param);
	nc[0]=0;
	}
    
    for ( b=0; b<n_param[field]; b++)
	       {
               
               local_table[field]=b;
	       if ( field<tot_n_param-1)
	          {
                  table=make_recursive_combination_table ( tot_n_param, n_param, nc, table, field+1);
		  }
	       else
	          {
                  for ( c=0; c< tot_n_param; c++)table[nc[0]][c]=local_table[c];
		  nc[0]++;
		  }
	       }
    return table;
    }
	          
/*********************************************************************/
/*                                                                   */
/*                         STRING PROCESSING                         */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
char *strnrchr ( char *s,char x, int n)
{
  int a;
  for (a=0; a< n; a++)if (s[a]=='x')return s+a;
  return NULL;
}
int intlen (int n)
{
  char buf [100];
  sprintf ( buf, "%d", n);
  return strlen (buf)+1;
}
char * update_string (char string1[], char string2[])
{
  if ( string1==string2);
  else if ( string2==NULL)
    {
      if ( string1==NULL)string1=vcalloc ( 1, sizeof(char));
      string1[0]='\0';
    }
  else
    {
      int l1, l2;
      l1=read_array_size_new (string1);
      l2=strlen (string2)+1;
      if (l1<l2)
	string1=vrealloc (string1, (l2)*sizeof (char));
      sprintf ( string1, "%s", string2);
    }
  return string1;
}
  
char* strcatf  (char *string1,char *string2, ...)
{
  
  va_list ap;
  char *buf;
  
  buf=vcalloc ( read_array_size_new (string1)+1, sizeof (char));
  va_start (ap, string2);
  vsprintf (buf, string2, ap);
  va_end (ap);
  string1=vcat (string1, buf);
  vfree (buf);
  return string1;
}

char *vcat ( char *st1, char *st2)
{
  int l=0;
  char *ns;
  
  if ( !st1 && !st2)return NULL;
  if ( st1) l+=strlen (st1);
  if ( st2) l+=strlen (st2);
  l++;
  
  ns=vcalloc ( l, sizeof (char));
  sprintf ( ns, "%s%s", (st1)?st1:"", (st2)?st2:"");
  return ns;
}
int print_param ( char *param, FILE *fp);
int print_param ( char *param, FILE *fp)
{
  static char **p;
  static int np;
  int a;
  
  if (!p)p=vcalloc (1000, sizeof (char*));
  
  for ( a=0; a<np; a++)if (p[a]==param) return 0;
  
  p[np++]=param;
  
  fprintf ( fp, "\nPARAMETERS: %s\n", param);
  return 1;
}
  
int strget_param ( char *string, char *token1, char *token2, char *format, ...)
{
  /*string: command line
    token1: parameter
    token2: default value (in a string)
    format: standard scanf format
    ... arguments for scanf
  */
  char *buf;
  char *buf2;
  int n;
  va_list ap;
  
  if (!string) return 0;
  //print_param (string, stdout);
  //print_param (string, stderr);
  va_start (ap, format);
  buf=after_strstr ( string, token1);
  
  
  if (buf)
    {
      buf2=vcalloc (strlen (buf)+1, sizeof (char));
      sprintf ( buf2, "%s", buf);
      buf2=substitute (buf2, "__", " ");
      n=my_vsscanf (buf2, format, ap);
      vfree (buf2);
    }
  else     {n=my_vsscanf (token2, format, ap);}
  va_end (ap);
  
  return n;
}

int strscanf (char *string1,char *token, char *format,...)
{
  char *buf;
  va_list ap;
  int n;
  
  va_start (ap,format);
  buf=after_strstr ( string1, token);
  if ( buf){n=my_vsscanf (buf, format, ap);}
  else n=0;
  
  va_end (ap);
  return n;
}

int match_motif ( char *string, char **motif)//crude string matching, the motif and the string have the same length
  {
  int l, a;
  if (!motif) return 0;
  l=strlen (string);
  for ( a=0; a<l; a++)
      {
	if ( motif[a][0]!='*' && !strchr (motif[a], string[a]))return 0;
      }
  return 1;
} 

char *after_strstr ( char *string, char *token)
{
  char *p;
  if ( (p=vstrstr (string, token)))return p+strlen (token);
  else return NULL;
}
char* lstrstr ( char *in, char *token)//return strstr if matches on the left
{
  char *s;
  if ((s=vstrstr(in, token))&&s==in)return s;
  else return NULL;
  
}

char *vstrstr ( char *in, char *token)
{
  if (!in || !token) return NULL;
  else return strstr (in, token);
}
char ** push_string (char *val, char **stack, int *nval, int position)
{
  char **new_stack;
  int a;
  
  
  if (!val || nval[0]<=position)return stack;
  nval[0]++;
  new_stack=vcalloc ( nval[0], sizeof (char*));  
  new_stack[position]=val;
  for (a=0; a< position; a++)new_stack[a]=stack[a];
  for (a=position+1; a<nval[0]; a++)new_stack[a]=stack[a-1];
  vfree (stack);
  
  return new_stack;
}

int vsrand (int val)
{
  static int initialized;
  
  if (initialized) return 0;
  else if (   getenv ("DEBUG_SRAND"))
    {
      srand (10);
      initialized=1;
    }
  else
    {
      
      /*Make sure two processes in a row do not get the same time value*/
      srand ((val==0)?time(NULL):val);
      initialized=1;
    }
  return 1;
}
int  *randomize_list (int *list, int len, int ncycle)
{
  int p1, p2, a, buf;
  
  vsrand (0);
  if ( ncycle==0)ncycle=len;
  for ( a=0; a<ncycle; a++)
    {
      p1=rand()%len;
      p2=rand()%len;
      buf=list[p1];
      list[p1]=list[p2];
      list[p2]=buf;
    }
  return list;
}
/*Replace by a gap the parts of the two strings that do not OVERLAP*/
/*returns the length of the overlap*/
int vstrcmp (const char *s1, const char *s2)
{
  if ( !s1 && !s2)return 0;
  else if ( !s1 || !s2)return 1;
  else return strcmp ( s1, s2);
}
int vstrncmp (const char *s1, const char *s2, int n)
{
  if ( !s1 && !s2)return 0;
  else if ( !s1 || !s2)return 1;
  else return strncmp ( s1, s2, n);
}
FILE *print_array_char (FILE *out, char **array, int n, char *sep)
        {
	int a;
	if ( array==NULL || read_size_char (array,sizeof (char*))<n)
	   {
	   myexit(fprintf_error ( stderr, "\nORB in print_array_char [FATAL]\n"));
	   crash("");
	   }
	for ( a=0; a< n; a++)fprintf ( out, "%s%s", array[a],sep);
	return out;
	}
char * path2filename ( char *array)
{
  Fname *F;
  char *name;

  name=vcalloc ( strlen (array)+2, sizeof (char));
  F=parse_fname (array);
  if ( F->suffix)sprintf ( name, "%s.%s", F->name, F->suffix);
  else sprintf (name, "%s", F->name);
  free_fname (F);
  return name;
}
Fname* parse_fname ( char *array)
	 {
	 int l;
	 Fname *F;



	 F=declare_fname (sizeof (array));
	 
	 sprintf ( F->full, "%s", array);
	 sprintf ( F->path, "%s", array);	
	 l=strlen (array);
	 while (l!=-1 && (F->path)[l]!='/')(F->path)[l--]='\0';
	 
	 sprintf ( F->name, "%s", array+l+1);
	 l=strlen (F->name);
	 while (l!=-1)
	   {
	    if((F->name)[l]=='.')
	      {
	      F->name[l]='\0';
	      sprintf ( F->suffix, "%s", F->name+l+1);
	      break;
	      }
	    else l--;
	    }

	return F;
        }
char *filename2path (char *name)
{
  char *nname;
  int x;
  if (isdir (name))return name;
  
  x=strlen (name)-1;
  nname=vcalloc (x+2, sizeof (char));
  sprintf ( nname, "%s", name);
  while ( x >=0 && nname[x]!='/')nname[x--]='\0';
  
  if ( !isdir (nname) || !nname[0]){vfree (nname); return NULL;}
  return nname;
}
  
  

  
    
char *extract_suffixe ( char *array)
        {
	int l;
	char *new_string;
	char *x;
	l=strlen (array);
	new_string=vcalloc ( l+1, sizeof (char));
	sprintf (new_string, "%s",array);
	
	x=new_string+l;
	while (x!=new_string && x[0]!='.' && x[0]!='/' )x--;
	if ( x[0]=='.')x[0]='\0';
	else if (x[0]=='/')return x+1;
 
	while ( x!=new_string && x[0]!='/')x--;
	
	return (x[0]=='/')?x+1:x;
	}
void string_array_upper ( char **string, int n)
     {
     int a;
     for ( a=0; a< n; a++)upper_string (string[a]);
     }
void string_array_lower ( char **string, int n)
     {
     int a;
     for ( a=0; a< n; a++)lower_string (string[a]);
     }

char *upper_string ( char *string)
	{
	int len, a;
	
	len=strlen ( string);
	for ( a=0; a< len; a++)string[a]=toupper ( string[a]);
	return string;
	}	
char *lower_string ( char *string)
	{
	int len, a;
	
	len=strlen ( string);
	for ( a=0; a< len; a++)string[a]=tolower ( string[a]);
	return string;
	}	
void string_array_convert ( char **array, int n_strings, int ns, char **sl)
        {
	int a;

	for ( a=0; a< n_strings; a++)string_convert ( array[a], ns, sl);
	}
void string_convert( char *string, int ns, char **sl)
        {
	int a, l;
	l=strlen ( string);
	for ( a=0; a< l; a++)
	    string[a]=convert(string[a], ns, sl);
	}
int convert ( char c, int ns, char **sl)
        {
	int a;
	int return_char;

	for ( a=0; a< ns; a++)
	    {
	    if ((return_char=convert2 ( c, sl[a]))!=-1)
		return return_char;
	    }
	return c;
	

	}
int convert2 ( char c, char *list)
    {
    int a;
    int l1;
    int return_char;

    l1=strlen ( list);
    
    return_char=(list[l1-1]=='#')?c:list[l1-1];

    for ( a=0; a< l1; a++)
	   if (list[a]=='#')return return_char;
           else if ( list[a]==c)return return_char;
    
    return -1;
    }
char* substitute_old ( char *string_in, char *t, char *r)
{
  char *string_out;
  char *p, *heap_in;
  int delta, l;
  /*REplaces every occurence of token t with token r in string_in*/

  if ( string_in==NULL || t==NULL || r==NULL) return string_in;
  
  heap_in=string_in;

  l=read_array_size_new ((void*)string_in)+1;

  string_out=vcalloc (l, sizeof (char));
  delta=strlen(r)-strlen (t);
  delta=(delta<0)?0:delta;
	 
  while ( (p=strstr ( string_in, t))!=NULL)
    {

      p[0]='\0';
      if ( delta)
	{
	  l+=delta; 
	  string_out=vrealloc(string_out, sizeof (char)*l);
	}
      
      strcat ( string_out, string_in);
      strcat ( string_out, r);
      string_in=p+strlen (t);
    }
  strcat ( string_out, string_in);
  if (l<strlen (string_out))
    {
      heap_in=vrealloc (heap_in, sizeof(char)*(strlen (string_out)+1));
    }
  sprintf ( heap_in, "%s", string_out);  
  vfree (string_out);
  return heap_in;
}
//Makes sure substitutions of the tild only occur on valid UNIX pathnames
char* tild_substitute ( char *string_in, char *t, char *r)
{
  char *p;
 
  p=strstr ( string_in, "~");
  if ( p==NULL)return string_in;
  else if (p && p!=string_in && p[-1]!='/')return string_in;
  
  else
    {
      return substituteN (string_in, t, r,1);
    }
  
}

char* substitute_char_set (char *s, char *set, char r)
{
  int l,a;

  if ( !s || !set || !r) return s;
  l=strlen (set);
  for (a=0; a<l; a++)
    s=substitute_char (s, set[a], r);
  return s;
}
    

char* substitute_char ( char *string_in, char t, char r)
{
  int n=0;
  while ( string_in && string_in[n]!='\0')
    {
      if ( string_in[n] == t)string_in[n]=r;
      n++;
    }
  return string_in;
}
char* substitute ( char *string_in, char *t, char *r)
{
  /*REplaces every occurence of token t with token r in string_in*/
  return substituteN(string_in, t, r,0);
}
char* substituteN ( char *string_in, char *t, char *r, int N)
{
  char *string_out;
  char *p, *heap_in, n=0;
  int delta, l, lsi,lso,lr,lt,nt;
 
  /*REplaces the first N Occurences*/
  if ( string_in==NULL || t==NULL || r==NULL) return string_in;
  
  heap_in=string_in;

  l=read_array_size_new ((void*)string_in)+1;
  lr=strlen(r);
  lt=strlen(t);
  lsi=strlen (string_in);
  
  delta=((lr-lt)>0)?(lr-lt):0;
  nt=0;
  while ( (p=strstr (string_in, t))!=NULL)
    {
      string_in=p+lt;
      nt++;
    }
  string_in=heap_in;
  
  lso=nt*delta+lsi;
  string_out=vcalloc (lso+1, sizeof (char));
  
  while ((N==0 ||n<N) && (p=strstr ( string_in, t))!=NULL)
    {
      p[0]='\0';
      strcat ( string_out, string_in);
      strcat ( string_out, r);
      string_in=p+lt;
      if (N!=0)n++;
    }
  strcat ( string_out, string_in);
  if (l<(lso+1)){HERE ("Realloc %s %s delta=%d %d %d", t, r,delta, lsi, lso);heap_in=vrealloc (heap_in, sizeof (char)*(lso +1));}
  sprintf ( heap_in, "%s", string_out);  
  vfree (string_out);
  return heap_in;
}

  
char **clean_string (int n, char **string)
{
  int a,b,c,l;
  if ( !string) return string;
  for (a=0; a< n; a++)
    {
      if (!string[a]) continue;
      l=strlen (string[a]);
      for (b=0; b<l; b++)
	{
	  c=string[a][b];
	  if (!isgraph(c) && c!='\n' && c!='\t' && c!=' ')string[a][b]=' ';
	}
    }
  return string;
}
	       



int str_overlap ( char *string1, char *string2, char x)
        {	
	int a, b;
	int l1, l2;
	int **array;
	int max1=0, max2=0;
	int score;
	int max_val=-1;
	int end1, end2;
	int start1, start2;

	if ( strm ( string1, string2))return 0;
	else
	    {
	    l1=strlen ( string1);
	    l2=strlen ( string2);
	    array=declare_int ( strlen ( string1), strlen ( string2));
	    for ( a=0; a< l1; a++)
		for ( b=0; b< l2; b++)
		    {
			if ( a==0 || b==0)array[a][b]=(string1[a]==string2[b])?1:0;
			else
			    if (string1[a]==string2[b])
			       {
			       score=array[a][b]=array[a-1][b-1]+1;
			       if ( max_val<score)
			          {
				  max_val=score;
				  max1=a;
				  max2=b;
				  }
			       }
		    }
	    start1=(max1+1)-max_val;
	    end1=  max1;
	    
	    start2=(max2+1)-max_val;
	    end2=  max2;
	    
	    for ( a=0; a< l1; a++)if ( a<start1 || a> end1)string1[a]=x;
	    for ( a=0; a< l2; a++)if ( a<start2 || a> end2)string2[a]=x;
	    
	    free_int ( array, l1);
	    
	    return max_val;
	    }
	}

int get_string_line ( int start, int n_lines, char *in, char *out)
	{
	int nl=0;
	int a=0;
	int c=0;
	
	while ( nl<n_lines)
		{
		while ( (c=in[start++])!='\n' && c!='\0')
			{
			out[a++]=c;
			}
		out[a++]='\n';
		nl++;
		}
	out[a]='\0';
	return (c=='\0')?-1:start;
	}
	

FILE * output_string_wrap ( int wrap,char *string, FILE *fp)
	{

	 int a, b,l;
	 
	 l=strlen ( string);
	 
	 for ( a=0, b=1; a< l; a++, b++)
	 	{
	 	fprintf ( fp, "%c", string[a]);
	 	if ( b==wrap)
	 		{
	 		fprintf ( fp, "\n");
	 		b=0;
	 		}
	 	}
	 return fp;
	 }

char * extract_char ( char * array, int first, int len)
    {
    char *array2;
    int a;

    len= ( len<0)?0:len;    

    array2=vcalloc ((len+1), sizeof (char));
    
    for ( a=0; a<len; a++)
	array2[a]=array[first++];

    array2[a]='\0';

    return array2;
    }    

int check_cl4t_coffee (int argc, char **argv)
{
  char *name;
  if ( name_is_in_list ("-other_pg", argv, argc, 100)!=-1)return 1;
  else if (name_is_in_list ( "tcoffee_test_seq.pep", argv, argc, 100)!=-1)return 1;
  else
    {
      int a,  inseq, result;
      FILE *fp;
      char command[10000];
      command[0]='\0';

      for ( inseq=0,a=0; a<argc; a++)
	{
	  if ( inseq && argv[a][0]=='-')
	    {
	      inseq=0;
	    }
	  else if (strm ( argv[a], "-seq"))inseq=1;

	  if ( inseq==0)
	    {
	      strcat ( command, " ");
	      strcat ( command, argv[a]);
	    }
	}
      name=vcalloc ( 100, sizeof (char));
      sprintf ( name, "TCIT_%d", (int)rand()%10000);
      strcat ( command, " -seq tcoffee_test_seq.pep -quiet -no_error_report");
      fp=vfopen ( name, "w");
      fprintf ( fp, ">A\nthecat\n>B\nthecat\n");
      vfclose (fp);
      result=safe_system (command);
      printf_system ( "rm %s.*", name);
      vfree (name);
      if (result) {myexit (EXIT_FAILURE);return 0;}
      else return 1;
    }
}
      
char** merge_list ( char **argv, int *argc)
       {
       int a, b;
       int n_in;
       char **out;
       char current [STRING];

       out=declare_char (argc[0], STRING);
       n_in=argc[0];
       argc[0]=0;
       
       a=0;
       while (a< n_in && !is_parameter ( argv[a]))
	 {
	   sprintf (out[argc[0]++], "%s",  argv[a]);
	   argv[a][0]='\0';
	   a++;
	 }
     
       
       for ( a=0; a< n_in; a++)
	 {
	   if ( is_parameter (argv[a]))
	     {
	       sprintf ( out[argc[0]++], "%s", argv[a]);
	       sprintf ( current, "%s", argv[a]);
	       
	       for ( b=0; b< n_in;)
		 {
		   if ( is_parameter (argv[b]) && strm (current, argv[b]))
			{
			  argv[b][0]='\0';
			  b++;
			  while ( b<n_in && !is_parameter ( argv[b]) )
			    {
			      if (argv[b][0])
			       {
				 sprintf ( out[argc[0]++], "%s", argv[b]); 
				 argv[b][0]='\0';
			       }
			     b++;
			    }
			}

		   else b++;
		     
		     
		   
		 }
	     }
	 }
  			  
       free_char (argv, -1);
       return out;
       }


int *  string2num_list_old ( char *string)
{
  /*Breaks down a list of numbers separated by any legal separator and put them in an array of the right size*/
  /*Returns list a list of integer*/
  /*Skips non numbers*/
  
  char *buf, *s;
  int  *list, n;
  

  buf=vcalloc ( strlen (string)+1, sizeof (char));
  
  n=0;
  sprintf ( buf, "%s", string);
  s=strtok (buf, SEPARATORS);
  while (s!=NULL)
	  {
	    n++;
	    s=strtok (NULL, SEPARATORS);
	  }
  list=vcalloc (n+1, sizeof (int));
  
  n=0;
  sprintf ( buf, "%s", string);
  s=strtok (buf, SEPARATORS);
  while (s!=NULL)
	  {
	    if (is_number(s))list[n++]=atoi(s);
	    s=strtok (NULL, SEPARATORS);
	  }
  vfree (buf);
 
  return list;
}



int *name_array2index_array ( char **list1, int n1, char **list2, int n2)
{
  int *list,a, max;
  /*returns an indexed list of the position of list1 in list2*/
  list=vcalloc ( n1, sizeof (int));
  for ( a=0, max=0; a< n1; a++)max=MAX(max, strlen (list1[a]));
  for ( a=0       ; a< n2; a++)max=MAX(max, strlen (list2[a]));
  
  for ( a=0; a< n1; a++)
    {
      list[a]=name_is_in_list (list1[a],list2,n2,max);
    }
  return list;
}

int * string2num_list ( char *string)
{
  return string2num_list2(string, SEPARATORS);
}
int * string2num_list2 ( char *string, char *separators)
{
   /*Breaks down a list of numbers separated by any legal separator and put them in an array of the right size*/
  /*Returns list a list of integer*/
  /*Skips non numbers*/
  
  char **nlist;
  int *list;
  int a, m, n;
  
  nlist=string2list2 (string, separators);
  
  if (nlist==NULL) return NULL;
  
  n=atoi(nlist[0]);
  list=vcalloc ( n, sizeof (int));
  
  for (m=1, a=1; a< n; a++)
    {
      if (is_number(nlist[a]))list[m++]=atoi(nlist[a]);
    }
  
  list[0]=m;
  free_char (nlist, -1);
  if ( m==0){vfree(list); return NULL;}
  else
    {
      return list;
    }
  
  return NULL;
  
}
char * list2string  ( char **list, int n)
{
  return list2string2 ( list, n, " ");
}
char * list2string2 ( char **list,int n, char* sep)
{
  int l, a;
  char *string;
  
  for ( l=0,a=0; a<n; a++)l+=strlen ( list[a])+1;
  string=vcalloc (l+1, sizeof (char));
  for ( a=0; a< n; a++)
      {
	strcat ( string, list[a]);
	strcat ( string, sep);
      }
    return string;
  }

char **  string2list ( char *string)
  {
    return string2list2(string, SEPARATORS);
  }
char **  string2list2 ( char *string, char *separators)
  {
  /*Breaks down a list of words separated by any legal separator and put them in an array of the right size*/
  /*Returns list a list of char with the size written in list[0]*/
  
  
  char *buf, *s;
  char  **list;
  int n, max_len;
  
  if ( string==NULL)return NULL;
  buf=vcalloc ( strlen (string)+2, sizeof (char));
  

  n=max_len=0;
  sprintf ( buf, "%s", string);
  s=strtok (buf, separators);
  
  while (s!=NULL)
	  { 
	    n++;
	    max_len=MAX(max_len,(strlen (s)));
	    s=strtok (NULL, separators);
	    
	  }
  
  if ( n==0){vfree(buf); return NULL;}
  
  list=declare_arrayN (2, sizeof (char), n+2, max_len+1);
    
  n=1;
  sprintf ( buf, "%s", string);
  s=strtok (buf, separators);
  while (s!=NULL)
	  {
	    sprintf (list[n++], "%s",s);
	    s=strtok (NULL, separators);
	  }
  
  sprintf (list[0], "%d", n);
  
  vfree (buf); 
  
  return list;
}

char** break_list ( char **argv, int *argc, char *separators)
       {
       int a, b;
       int n_in;
       char **out;
       char **ar=NULL;
       int n_ar;
       int cont=1;

       /*Breaks down the argv command line in smaller units, breaking at every separator*/
       out=vcalloc (MAX_N_PARAM, sizeof (char*));
       n_in=argc[0];
       argc[0]=0;
       
       if ( n_in>=MAX_N_PARAM)
	 {
	   myexit(fprintf_error ( stderr, "\nERROR: too many parameters, recompile with MAX_N_PARAM set at a higher velue [FATAL:%s]\n", PROGRAM));\
	   myexit (EXIT_FAILURE);
	 }
       
       for ( a=0; a< n_in; a++)
           {
	     

	     
	     if (cont)ar=get_list_of_tokens( argv[a], separators,&n_ar);
	     else ar=get_list_of_tokens( argv[a],"",&n_ar);


	     for ( b=0; b< n_ar; b++)
	       {
		 out[argc[0]]=vcalloc( strlen (ar[b])+1, sizeof (char));
		 sprintf (out[argc[0]++], "%s", ar[b]);
	       }
	     free_char (ar, -1);
	     ar=NULL;
	     if ( strstr (argv[a], "-other_pg"))cont=0;
	   }
       free_char (ar, -1);
       return out;
       }
    
char *invert_string2 (char *string)
{
  char *buf;
  int a, b, l;
  
  l=strlen (string);
  buf=vcalloc ( l+1, sizeof (char));
  for ( a=l-1, b=0; a>=0; a--, b++)
    buf[b]=string[a];
  sprintf (string, "%s", buf);
  vfree (buf);
  return string;
}
char *invert_string (char *string)
{
  return string2inverted_string(string);
}
char* string2inverted_string(char *string)
{
  char *buf;
  int a, b, l;
  
  l=strlen (string);
  buf=vcalloc ( l+1, sizeof (char));
  for ( a=l-1, b=0; a>=0; a--, b++)
    buf[b]=string[a];
  return buf;
}

char ** get_list_of_tokens ( char *in_string, char *separators, int *n_tokens)
{
    char **list=NULL;
    char *p=NULL;
    char *string;
    
    
    n_tokens[0]=0;
    if ( in_string==NULL || strm(in_string, ""));
    else if ( in_string[0]=='[')
      {
	list=declare_char (1, strlen ( in_string)+1);
	sprintf ( list[n_tokens[0]], "%s",in_string);
	n_tokens[0]++;
      }
    else
      {
	list=declare_char (strlen ( in_string)+1, 1);	
	string=vcalloc ( strlen(in_string)+1, sizeof (char));
	sprintf ( string, "%s", in_string);	
	
	while ( (p=strtok ((p==NULL)?string:NULL, ((separators==NULL)?SEPARATORS:separators)))!=NULL)
           {
	   list[n_tokens[0]]=vrealloc ( list[n_tokens[0]], sizeof (char) *strlen (p)+1);
	   sprintf ( list[n_tokens[0]], "%s", p);
	   n_tokens[0]++;
	   }

	vfree (string);
	}
   return list;
   }

char **ungap_array ( char **array, int n)
        {
	int a;
	for ( a=0; a< n; a++)ungap(array[a]);
	return array;
	}

void ungap ( char *seq)
{
  remove_charset ( seq, "ungap");
}
int seq2len (char *seq, char *pset,char *nset)
{
  int a, l, t=0;
  //count all the residues in pset and NOT in nset
  if ( !seq) return 0;
  
  l=strlen (seq);
  //returns the len of the string 
  for (a=0; a< l; a++)
    {
      char c=seq[a];
      if ( pset && nset && strchr (pset, c) && !strchr (nset, c))t++;
      else if ( pset && strchr  (pset, c))t++;
      else if ( nset && !strchr (nset, c))t++;
    }
  return t;
}
int seq2res_len (char *seq)
{
  return seq2len (seq, NULL, GAP_LIST);
}
char* remove_charset_from_file (char *fname, char *set)
{
  char *tmp;
  char c;
  FILE *fp1;
  FILE *fp2;
  
  fp1=vfopen (fname, "r");
  fp2=vfopen (tmp=vtmpnam (NULL), "w");
  while ( (c=fgetc(fp1))!=EOF)
    {
    if (!strchr ( set,c))fprintf ( fp2, "%c", c);
    }
  vfclose (fp1);
  vfclose (fp2);
  return tmp;
}
  
void remove_charset ( char *seq, char *set)
	{
	int a, b, l;
	char *set2;

	set2=vcalloc (256, sizeof (char));
	if ( strm (set, "!alnum"))
	  {
	    for ( b=0,a=1;a< 256; a++)if ( !isalnum (a))set2[b++]=a;
	  }
	else if ( strm ( set, "ungap"))
	  {
	    sprintf ( set2, "%s", GAP_LIST);
	  }
	else
	  {
	    sprintf ( set2, "%s", set);
	  }
	
	l=strlen ( seq);
	for (b=0, a=0; a<=l; a++)
	  {
	    if ( strchr ( set2, seq[a]));
	    else seq[b++]=seq[a];
	  }
	seq[b]='\0';
	vfree (set2);
	}

  
char **char_array2number ( char ** array, int n)
       {
       int a;
       for ( a=0; a< n; a++)array[a]=char2number(array[a]);
       return array;
       }
char *char2number ( char * array)
       {
       int a, l;
       
       
       l=strlen ( array);
       for ( a=0; a< l; a++)
	   {
	     if ( isdigit(array[a]) && array[a]!=NO_COLOR_RESIDUE && array[a]!=NO_COLOR_GAP )array[a]-='0';
	     else if ( array[a]<9);
	     else if ( array[a]==NO_COLOR_RESIDUE || array[a]==NO_COLOR_GAP)array[a]=NO_COLOR_RESIDUE;
	   }
       return array;
       }
long atop (char*p)
{
  /*turns a char into a pointer*/
  if ( p==NULL) return 0;
  else return atol(p);
}

char *mark_internal_gaps(char *seq, char symbol)
{
  int l, a, gap;
  int in_seq;
  char *cache_seq;
  
  l=strlen(seq);
  cache_seq=vcalloc ( l+1, sizeof (char));
  sprintf ( cache_seq, "%s", seq);
  
  for ( gap=0, in_seq=0,a=0; a< l; a++)
    {
      gap=is_gap(seq[a]);
      if ( !gap && !in_seq)in_seq=1;
      if (gap && in_seq)seq[a]=symbol;      
    }
  
  for (gap=0, in_seq=0,a=l-1; a>=0; a--)
    {
      gap=is_gap(seq[a]);
      if ( !gap && !in_seq)break;
      if (gap && !in_seq)seq[a]=cache_seq[a];
    }
  vfree(cache_seq);
  return seq;
}

void splice_out ( char *seq, char x)
    
        {
	int a, b, l;

	l=strlen ( seq);
	for (b=0, a=0; a<=l; a++)
		if ( seq[a]==x);
		else seq[b++]=seq[a];	
	seq[b]='\0';
	}
char *splice_out_seg ( char *seq, int pos, int len)
{
  int l, a;
  
  if (seq==NULL || pos<0) return seq;
  l=strlen (seq);
  if ( l<(pos+len))
    printf_exit ( EXIT_FAILURE, stderr, "Splice_out_seg out of bound: Length %d seg: [%d %d] [splice_out_seg::util.c][FATAL:%s]\n", l, pos, pos+len, PROGRAM);
  l-=len;
  for (a=pos; a< l; a++)
    seq[a]=seq[a+len];
  seq[a]='\0';
  return seq;
}
  
int isblanc ( char *buf)
   {
   int a, l;
   
   if ( buf==NULL)return 0;
   l=strlen (buf);
   for ( a=0; a< l; a++)
   	if (isalnum (buf[a]))return 0;
   return 1;
   }



int is_number  ( char *num)
   {
     int a, l;
     l=strlen (num);
     
     for (a=0;a<l; a++)
       if ( !strchr ("0123456789.-+", num[a]))return 0;
     return 1;
   }

int is_alnum_line ( char *buf)
   {
   int a, l;
   l=strlen (buf);
   for ( a=0; a< l; a++)
   	if (isalnum (buf[a]))return 1;
   return 0;
   } 
int is_alpha_line ( char *buf)
   {
   int a, l;
   l=strlen (buf);
   for ( a=0; a< l; a++)
   	if (isalpha (buf[a]))return 1;
   return 0;
   }
int case_insensitive_strcmp ( char *string1, char *string2)
{
  int a;
  int l;
  
  if ( !string1 && !string2) return 1;
  else if ( !string1 && !string2) return 0;
  else
    {
      l=strlen (string1);
      for ( a=0; a< l; a++)
	{
	  if (tolower(string1[a])!=tolower(string2[a]))return 0;
	}
    }
  return 1;
}	
int get_string_sim ( char *string1, char *string2, char *ignore)
	{
	int len1;
	int len2;
	int a;
	int pos=0;
	int sim=0;
	char r1, r2;
	
	
	len1=strlen (string1);
	len2=strlen (string2);
	
	if ( len1!=len2)return 0;
	
	for ( a=0; a< len1; a++)
		{
		r1=string1[a];
		r2=string2[a];
		if ( !is_in_set (r1, ignore) && !is_in_set (r2, ignore))
			{
			pos++;
			if (r1==r2)
				{
				sim++;
				}
			}
		}
	
	if (pos==0)
		 return 0;
	else	
		return (int) (sim*100)/pos;
	
	}
int haslower (char *s)
{
  if (!s) return 0;
  else
    {
      int l,a;
      l=strlen (s);
      for (a=0; a<l; a++)if (islower(s[a]))return 1;
      return 0;
    }
}
int hasupper (char *s)
{
  if (!s) return 0;
  else
    {
      int l,a;
      l=strlen (s);
      for (a=0; a<l; a++)if (isupper(s[a]))return 1;
      return 0;
    }
}

int is_aa  ( char x)
         {
	 return (is_in_set (x, AA_ALPHABET) && !is_in_set (x, GAP_LIST));
	 }
int is_rna ( char x)
         {
	 return (is_in_set (x, RNAONLY_ALPHABET)&& !is_in_set (x, GAP_LIST));
	 }
int is_dna ( char x)
         {
	 return (is_in_set (x, DNA_ALPHABET)&& !is_in_set (x, GAP_LIST));
	 }
int is_gap ( char x)
         {
	 return ( is_in_set ( x, GAP_LIST));
	 }
int is_gop ( int p, char *s)
{
  if (!s) return 0;
  else if (p<=0)return 0;
  else if (s[p]!='-' && s[p+1]=='-')return 1;
  else return 0;
}
    
char* get_alphabet   ( char *seq, char *alphabet)
/*This function builds an alphabet from the string seq*/
/*Alphabet does not need to be emppty. The total number of unique symbols is returned*/
/*Gaps, as defined in the GAP_LIST are ignored*/
    {
	int a;
	int len;
	
	if ( !alphabet) alphabet=vcalloc ( 200, sizeof (char));

	len=strlen (seq);
	
	for ( a=0; a<len; a++)
	  if ( !is_gap ( seq[a]) && !is_in_set ( seq[a], alphabet+1)){alphabet[(int)++alphabet[0]]=seq[a];}
	return alphabet;
    }

int array_is_in_set ( char *x, char *set )
         {
	 int len;
	 int a;
	 
	 len=strlen (x);
	 for ( a=0; a< len; a++)
	     if ( !is_in_set ( x[a], set))return 0;
	 return 1;
	 }


int is_in_set ( char r, char *list)
	{
	static char s[2];
	
	s[0]=r;

	
	if ( strstr ( list,s)!=NULL)return 1;
	
	return 0;
	}
char * generate_void ( int x)
    {
    return generate_string (x, ' ');
    }
   
char * generate_null ( int x)
    {
    return generate_string ( x, '-');
    }
    	 	
char * generate_string ( int x, char y)
    {
	 int a;
    char *string;
    
    string = vcalloc ( x+1, sizeof ( char));

    for ( a=0; a< x; a++)
	string[a]=y;

    string[a]='\0';
    return string;
    }
char * translate_string (char *string, char *in, char*out)
{
  int l1, l2;
  int a, b;

  l1=strlen(string);
  l2=strlen (in);

  if ( l2!=strlen (out))
    {
      myexit(fprintf_error ( stderr, "\nERROR: translate_string function: IN alphabet (%s) must have the same size as OUT alphabet (%s) [FATAL:%s]\n", in, out, PROGRAM));
      myexit (EXIT_FAILURE);
    }
  
  for ( a=0; a< l1; a++)
    {
      for ( b=0; b< l2; b++)
	{
	  if ( string[a]==in[b])
	    {
	      string[a]=out[b];
	      b=l2;
	    }
	}
    }
  return string;
}
  

int get_longest_string (char **array,int n, int *len, int *index)
     {
     int a, l;
     int max_len=0, max_index=0;

     if ( n==0 || array==NULL )return 0;     
     if ( n==-1)n=read_size_char(array,sizeof (char*));
     

     if (read_size_char ( array,sizeof (char*))<n)
         {
	 myexit(fprintf_error ( stderr, "\nBAD REQUEST: Number of strings=%d, expected number=%d", read_size_char ( array,sizeof (char*)),n));
	 crash ("[FATAL/util.c]");
	 }
     else
         {
	 max_len=strlen(array[0]);
	 max_index=0;
	 for ( a=1; a< n; a++)
	     {
	     l=strlen ( array[a]);
	     if ( l>max_len)
	        {
		max_len=l;
		max_index=a;
		}
	     }
	 if (index!=NULL)index[0]=max_index;
	 if (len!=NULL)len[0]=max_len;
	 }
     
     return max_len;
     }

int get_shortest_string (char **array,int n, int *len, int *index)
     {
     int a, l;
     int min_len;

     if ( n==0|| array==NULL || read_size_char ( array,sizeof (char*))<n)return 0;
     else
         {
	 min_len=strlen(array[0]);

	 for ( a=1; a< n; a++)
	     {
	     l=strlen ( array[a]);
	     if ( l<min_len)
	        {
		min_len=l;

		}
	     }
	 if (index!=NULL)index[0]=a;
	 if (len!=NULL)len[0]=min_len;
	 }
     return min_len;
     }

char **pad_string_array ( char **array, int n, int len, char pad)
     {
     int a, b, l;
     for ( a=0; a< n; a++)
         {
	 l= strlen ( array[a]);
	 for (b=l; b<len; b++)array[a][b]=pad;
	 array[a][len]='\0';
	 }
     return array;
     }
char **right_pad_string_array ( char **array, int n, int len, char pad)
     {
     return pad_string_array(array, n, len, pad);
     }
char **left_pad_string_array ( char **array, int n, int len, char pad)
     {
     int a, b, c, l;
     char *buf;

     buf=vcalloc ( len+1, sizeof (char));
     for ( a=0; a< n; a++)
         {
	 l= strlen ( array[a]);
	 for (b=0; b<(len-l); b++)buf[b]=pad;
	 for (c=0,b=(len-l); b<=len; c++,b++)buf[b]=array[a][c];	 
	 sprintf (array[a], "%s", buf);

	 }
     vfree(buf);
     return array;
     }
char * crop_string (char *string, int start, int end)
     {
     static char *buf;
     /*Extract [start-end[*/

     if ( strlen (string)<end)
             {
	     myexit(fprintf_error ( stderr, "\nString=%s, start=%d end=%d", string, start, end));
	     crash ( "Wrong End in crop String [FATAL]");
	     }
     else
         {
	 buf=vcalloc (strlen (string)+1, sizeof (char));
	 string[end]='\0';
	 sprintf ( buf, "%s", string+start);
	 sprintf ( string,"%s", buf);
	 vfree (buf);
	 }

     return string;
     }
		  
int get_distance2char ( char *x, char*list)
    {

    char *y;
    y=x;
    if (x==NULL) return 0;
    while (!is_in_set (y[0], list) && y[0]!='\0')y++;
    return y-x;
    }
/*********************************************************************/
/*                                                                   */
/*                         TIME        FUNCTIONS                     */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
static long ref;
static int child;
static long ticks;
static long milli_sec_conv=1000;


FILE *print_program_information (FILE *fp, char *comment)
{
   fprintf ( fp, "# Results Produced with %s (%s)\n",PROGRAM, VERSION);
   fprintf ( fp, "# %s is available from %s\n",PROGRAM,URL);  
   if (comment)fprintf ( fp, "# %s\n", comment);
   return fp;
}
FILE* print_cpu_usage (FILE *fp, char *comment)
{
  fprintf ( fp, "# %s CPU Usage: %d millisec\n", comment, get_time());
  return fp;
}
void print_exit_success_message ()
{
  return;
  fprintf ( stderr, "\n# TERMINATION STATUS: SUCCESS [PROGRAM: %s pid %d ppid %d]\n#CL: %s\n", PROGRAM, getpid(), getppid(),in_cl);
}
void print_exit_failure_message ()
{
  fprintf ( stderr, "\n# TERMINATION STATUS: FAILURE [PROGRAM: %s pid %d ppid %d\n#CL: %s\n", PROGRAM, getpid(), getppid(), in_cl);
}




int get_time ()
	{
	static long time;
        struct tms time_buf[1];
	long tms_stime, tms_utime;

	if ( child==1)return get_ctime();
	  
	if ( ticks==0)ticks = sysconf(_SC_CLK_TCK);
	times ( time_buf);

	tms_stime=(long)time_buf->tms_stime*milli_sec_conv;
	tms_utime=(long)time_buf->tms_utime*milli_sec_conv;
	


	
	if ( ref==0)
		{
		ref=(tms_stime+tms_utime);
		return 0;
		}
	else
		{
		time=(tms_utime+tms_stime)-ref;
		return (int) ((time)/ticks);
		}
	}
int get_ctime ()
	{
	static long time;
        struct tms time_buf[1];
	long   tms_cutime, tms_cstime; 
	
	if ( ticks==0)ticks = sysconf(_SC_CLK_TCK);
	times ( time_buf);



	tms_cstime=(long)time_buf->tms_cstime*milli_sec_conv;
	tms_cutime=(long)time_buf->tms_cutime*milli_sec_conv;

	if ( ref==0)
	        {
		child=1;
		ref=tms_cstime+tms_cutime;
		return 0;
		}
	else
		{
		time=(tms_cutime+tms_cstime)-ref;
		return (int)((time)/ticks);
		}
	}
int reset_time()
        {
	ref=0;
	return (int)get_time();
	}
int increase_ref_time(int increase)
        {
	if ( ref==0)get_time();
	
	ref-=(long)ticks*(long)increase;
	if (ref==0)ref++;
	return (int)ref;
	}

/*********************************************************************/
/*                                                                   */
/*                         SYSTEM CALLS                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/	
int evaluate_sys_call_io ( char *out_file, char *com, char *fonc)
     {
       if ( file_exists (NULL,out_file))return 1;
       else 
	 {
	   add_warning (stderr, "COMMAND FAILED: %s",com); 
	   return 0;
	 }
     }
void HERE (char *string, ...)
{
  va_list ap;
  
  va_start (ap, string);
  fprintf ( stderr, "HERE: ");
  vfprintf (stderr, string, ap);
  fprintf ( stderr, "\n");
  va_end (ap);
  
}
//defined this way because all compilers cannot pas ap
//safe printf: it declares buf to the proper size
#define cvsprintf(buf,string)\
if(1)\
  {						\
    va_list ap;					\
    int n;					\
    char buf2[2];				\
    va_start (ap,string);			\
    n=vsnprintf (buf2,1, string, ap)+2;		\
    va_end (ap);				\
    va_start (ap, string);			\
    buf=vcalloc (n, sizeof (char));		\
    vsnprintf (buf, n,string, ap);		\
    va_end(ap);}
    
int fprintf_fork  (FILE *fp, char *string, ...)
{
  static char *openF;
  static char *closeF;
  
  char *pid_file;
  FILE *flag;
  struct flock fl;
  int fd,a;

  char *buf;
 
  if (!openF)
    {
      openF=vcalloc (100, sizeof (char));
      sprintf (openF, "cedric1");
      closeF=vcalloc (100, sizeof (char));
      sprintf (closeF, "cedric2");
      
      //openF =vtmpnam (NULL);
      //closeF=vtmpnam (NULL);
      vfclose(vfopen (openF,"w"));
    }
  while ((rename (openF,closeF))==-1);
    
  cvsprintf (buf,string);
  fprintf ( fp, "%s", buf);
  fflush (fp);
  rename (closeF, openF);
  vfree (buf);
  return 0;
}
int fprintf_fork2  (FILE *fp, char *string, ...)
{

  static char *openF;
  static struct flock fl;
  char* buf;
  int fd;
  cvsprintf (buf, string);
  fprintf ( fp, "%s", buf);
  vfree (buf);
  fflush (fp);
  return 0;
}

int printf_file  (char *file,char *mode, char *string,...)
{
  FILE *fp;
  va_list ap;
  
  if (!(fp=vfopen (file, mode)))return 0;
  va_start (ap, string);
  vfprintf (fp, string, ap);
  va_end (ap);
  vfclose (fp);
  return 1;
  }
int printf_system_direct  (char *string, ...)
{
  char *buf;
  int r;
  
  cvsprintf (buf, string);
  r=system (buf);
  vfree(buf);
  return r;
}

int printf_system  (char *string, ...)
{
  char *buf;
  int r;
  
  cvsprintf (buf, string);
  r=my_system (buf);
  vfree(buf);
  return r;
}

int my_system_cl (int argc, char *argv[])
{
  int a,l;
  char *command;
  
  for ( a=0, l=0; a< argc; a++)l+=(strlen(argv[a])+2);
  command=vcalloc (l+1, sizeof(char));
  for ( a=0; a< argc; a++)
    {
      command=strcat (command, argv[a]);
      command=strcat (command, " ");
    }
  a=my_system ( command);
  vfree (command);
  return a;
}

int my_system ( char *command0)
{
  static char ***unpacked_list;
  static int n_unpacked;
    
    if (!unpacked_list)
    {
      unpacked_list=declare_arrayN(3, sizeof (char), 3, 200,300);
    }

  if ( getenv ("DEBUG_PERL"))return safe_system (command0);
  else
    {
      char **list;
      int is_command;
      int a, c=0;
      char *command1;
      char *command2;
      int return_val;
      
      command1=vcalloc ( 3*strlen (command0)+1, sizeof (char));
      command2=vcalloc ( 100000, sizeof (char));      
      sprintf ( command1, "%s", command0);
      
      command1=substitute (command1, "|", " | ");
      command1=substitute (command1, ";", " ; ");
      
      list=string2list (command1);
      if ( !list) return EXIT_SUCCESS;
      is_command=1;

      //Identify T-Coffee self threads and install threads
            
      for ( a=1; a< atoi(list[0]); a++)
	{
	  if ( is_command)
	    {
	      if ( strstr ( list[a], "unpack_"))
		{
		  unpack_all_perl_script (list[a]+strlen ("unpack_"));
		  myexit (EXIT_SUCCESS);
		}
	      else if ((c=name_is_in_list (list[a], unpacked_list[0], n_unpacked, 100))!=-1);
	      else
		{
		  n_unpacked=unpack_perl_script (list[a], unpacked_list, n_unpacked);c=n_unpacked-1;
		}
	      //if non unpacked script check pg is installed:
	   	      
	      if ( strm (unpacked_list[2][c], "shell"))
		{
		  check_program_is_installed (list[a], NULL, NULL, NULL, INSTALL_OR_DIE);
		}
	      strcat (command2, ((c!=-1)?unpacked_list[1][c]:list[a]));
	      strcat (command2, " ");
	      is_command=0;
	      
	    }
	  else
	    {
	      strcat (command2, list[a]);
	      strcat (command2, " ");
	      if ( strm (list[a], ",") ||strm (list[a], "|")) is_command=1;
	    }
	}
      
      free_char (list,-1);
      vfree ( command1);
      command2=substitute ( command2, "//", "/");
      
      return_val=safe_system (command2);
      
      vfree ( command2);
      return return_val;
    }
}
int has_warning_lock()
{
  if (lock(getpid(), LWARNING, LCHECK,NULL))return 1;
  else return 0;
}
int has_error_lock()
{
  if (lock(getpid(), LERROR, LCHECK,NULL))return 1;
  else return 0;
}
int is_shellpid(int pid)
{
  if ( lock(pid, LLOCK, LCHECK, NULL) && strstr (lock(pid,LLOCK, LREAD, NULL), "-SHELL-"))return 1;
  else return 0;
}
int is_rootpid()
{
  int r;
  if (debug_lock)
    {
      fprintf ( stderr,"\n\t------ check if %d isrootpid (util): %s->%d", getpid(),lock2name (getppid(),LLOCK), (lock(getppid(), LLOCK, LCHECK, NULL))?1:0);
    }
		
  
  if(lock (getppid(), LLOCK, LCHECK, NULL)!=NULL)return 0;
  else return 1;
}

int shift_lock ( int from, int to, int from_type,int to_type, int action)
{
  //action: SET (concatenate) or RESET (replace parent with child content)
  char *e;
  if (!lock (from,from_type, LCHECK, NULL))return 0;
  e=lock (from,from_type, LREAD, NULL);
  lock (from,from_type, LRELEASE, NULL);
  
  if ( action==LSET || action==LRESET)lock (to, to_type,action, e);
  else 
    {
      myexit(fprintf_error (stderr, "Unsupported type for shift_lock"));
    }
  vfree (e);
  return 1;
}

char*lock2name (int pid, int type)
{
  static char *fname;
  char host[1024];
  gethostname(host, 1023);
  
  vfree (fname);
  fname=vcalloc (strlen(host)+strlen (get_lockdir_4_tcoffee())+1000, sizeof (char));
  if (type == LLOCK)sprintf (fname, "%s/.%d.%s.lock4tcoffee",get_lockdir_4_tcoffee(), pid,host);
  else if ( type == LERROR) sprintf (fname, "%s/.%d.%s.error4tcoffee", get_lockdir_4_tcoffee(),pid,host); 
  else if ( type == LWARNING) sprintf (fname, "%s/.%d.%s.warning4tcoffee",get_lockdir_4_tcoffee(),pid,host); 
  else myexit(fprintf_error ( stderr, "ERROR: Unknown type for lock"));
  return fname;
}

char* lock(int pid,int type, int action,char *string, ...)
{
  char *fname;
  
  char *r;

  
  fname=lock2name (pid, type);
  
  if (debug_lock)
    {
      fprintf (stderr,"\n\t\t---loc4tc(util.h) %d =>%s [RD: %s]\n", action, fname, getcwd(NULL, 0));
    }
  
  if (action == LREAD)
    {
      return file2string (fname);
    }
  else if ( action == LCHECK)
    {
      return (file_exists (NULL,fname))?"x":NULL;
    }
  else if (action== LRELEASE) 
    {
      
      if (debug_lock)
	{
	  printf_system_direct ("mv %s %s.released", fname, fname);
	}
      else if (file_exists (NULL, fname))
	{
	  vremove (fname);
	//safe_remove (fname);return NULL;
	}
    }
  else if ( clean_exit_started) return NULL; //NO MORE LOCK SETTING during EXIT Phase
  else if (action== LSET || action == LRESET)    
    {
      char *value;
      char *r;
      if (string)
	{
	  cvsprintf (value,string);
	}
      else
	{
	  value=vcalloc (2, sizeof(char));
	  sprintf (value, " ");
	}
      string2file (fname, (action==LSET)?"a":"w", value);
      vfree (value);
      return " ";
    }
  else myexit(fprintf_error ( stderr, "ERROR: Unknown action for LOCK"));
  
  return NULL;
    
}
int check_process (const char *com,int pid,int r, int failure_handling)
{
  char *error;
  //If the child process has an error lock, copy that lock into the parent'lock
  //The error stack trace of the child gets passed to the parent
  if (debug_lock)fprintf (stderr, "\nEVAL_CALL ----- %d ->%s\n",pid, (r==EXIT_FAILURE)?"FAIL":"SUCCESS");
  
  if ( failure_handling == IGNORE_FAILURE) return r;
  if ( lock(pid, LWARNING, LCHECK, NULL))
    {
      shift_lock (pid, getpid(), LWARNING, LWARNING,LSET);
    }

  
  if ( lock(pid, LERROR, LCHECK, NULL))
    {
      shift_lock (pid, getpid(), LERROR,LERROR, LSET);
    }
  else if (r==EXIT_FAILURE)
    {
      //Reconstruct missing errorlock
      lock (getpid(), LERROR,LSET,"%d -- ERROR: UNSPECIFIED UNSPECIFIED\n",pid);
      lock (getpid(), LERROR,LSET,"%d -- COM: %s\n",pid,com);
      lock (getpid(), LERROR, LSET,"%d -- STACK: %d -> %d\n",getpid(), pid);
    }
  //lock is now ready. Shall we use it?

  if (lock(getpid(), LERROR, LCHECK, NULL))
    {
      if (failure_handling==RETURN_ON_FAILURE)
	{
	  shift_lock(getpid(),getpid(),LERROR, LWARNING,LSET);
	}
      else
	{
	  myexit (EXIT_FAILURE);
	}
    }
  return r;
}
  

int safe_system (const char * com_in)
{
    pid_t pid;
    int   status;
    int return_value;
    int failure_handling;
    char *p;
    char command[1000];
    
    static char *com;

    if ( clean_exit_started) return system (com_in);
    if ( com)vfree (com);
    if ( strstr ( com_in, "SCRATCH_FILE"))
      {
	com=vcalloc ( strlen ( com_in)+1, sizeof (char));
	sprintf ( com, "%s", com_in);
	while (strstr ( com, "SCRATCH_FILE"))
	  {
	    char *t;
	    t=vtmpnam(NULL);
	    com=vrealloc (com, (strlen (com)+strlen (t)+1)*sizeof (char));
	    
	    
	    com=substitute (com,"SCRATCH_FILE", t);
	  }
      }
    else
      {
	com=vcalloc (strlen (com_in)+1, sizeof (char));
	sprintf ( com, "%s", com_in);
      }
    

    if (com == NULL)
        return (1);
    else if ( (p=strstr (com, "::IGNORE_FAILURE::")))
      {
	p[0]='\0';
	failure_handling=IGNORE_FAILURE;
      }
    else if ( (p=strstr (com, "::RETURN_ON_FAILURE::")))
      {
	p[0]='\0';
	failure_handling=RETURN_ON_FAILURE;
      }
    else
      {
	failure_handling=EXIT_ON_FAILURE;
      }
    
	      
    sprintf ( command, " -SHELL-%s (tc)", com_in);
    if ((pid = vvfork (command)) < 0)
        return (-1);
    
    if (pid == 0) 
      {
	char * argv [4];
        
        argv [0] = "sh";


        argv [1] = "-c";
        argv [2] =(char*) com;
        argv [3] = 0;
	
	if ( debug_lock)fprintf (stderr,"\n--- safe_system (util.h): %s (%d)\n", com, getpid());
	execvp ("/bin/sh", argv);
      }
    else
      {
	set_pid(pid);
      }
    
    
    while (1) 
      {
	int r;
	r=vwaitpid (pid, &status, 0);
	
	
	if (errno ==EINTR)r=EXIT_SUCCESS;
	else if (r==-1 || status != EXIT_SUCCESS)r=EXIT_FAILURE;
	else r=EXIT_SUCCESS;
	if ( debug_lock)
	  fprintf ( stderr, "\n--- safe system return (util.c): p:%d c:%d r:%d (wait for %d", getppid(), getpid(), r, pid);  
	return check_process (com_in,pid,r, failure_handling);
      }
} 





static int **pidtable;
static int pidtable_s;
static int max_pid;
pid_t **realloc_pidtable (int value);
pid_t **realloc_pidtable (int value)
{
  int a;
  int new_max;

  new_max=value+10000;
  
  pidtable=vrealloc (pidtable, sizeof (pid_t*)*(new_max));
  for (a=max_pid; a<new_max; a++)
    pidtable[a]=vcalloc (2, sizeof (pid_t));
  max_pid=new_max;
  return pidtable;
}

  
pid_t **declare_pidtable ()
{
  int a;
  if (!max_pid)max_pid=40000;
  
  pidtable_s=MAX_N_PID;
  
  pidtable=vcalloc (pidtable_s, sizeof (pid_t*));
  for (a=0; a< pidtable_s; a++)
    {
      pidtable[a]=vcalloc (2, sizeof (pid_t));
    }
  return pidtable;
}
pid_t set_pid (pid_t p)
{
  int cpid;
  
  if (!pidtable)declare_pidtable();
  if ( p<=0) return;
  else if ( p>=max_pid)realloc_pidtable((int)p);
  pidtable[(int)p][0]=getpid();
  pidtable[(int)p][1]=1;
}
pid_t vvfork (char *type)
{
  pid_t p;
  static int attempt;
    
  p=fork();
  if (p==-1)
    {
      attempt++;
      if ( attempt==1000) 
	{
	  myexit(fprintf_error (stderr,"FORK_ERROR fork returned -1"));
	  return -1;
	}
      else 
	add_warning (stderr, "Could Not Fork %s [%d/%d tries]", PROGRAM, attempt, 1000);
      wait((pid_t*)-1);
      return vvfork(type);
    }
  else
    {
      attempt=0;
      if (p!=0)
	{
	  lock (getpid(), LLOCK, LSET, "%d\n",p);//Update the parent lock
	  set_pid(p);
	  return p;
	}
      else
	{
	  lock (getpid(), LLOCK, LSET, "%d%s\n", getppid(), (type)?type:"");//Create lock for the fork
	  
	  if (debug_lock)fprintf ( stderr, "\nFORKED (util): p=%d child=%d\n", getppid(), getpid());
	  
	  return 0;
	}
    }
}
int    vwait_npid (int sub, int max, int min)
{
  if (max==0)
    {
      while (sub>0)
	{
	  vwait (NULL);
	  sub--;
	}
    }
  else if ( sub>=max)
    {
      while (sub>=min)
	{
	  vwait (NULL);
	  sub--;
	}
    }
  else{;}
  return sub;
}
    
pid_t  vwaitpid (pid_t p, int *status, int options)
{
  pid_t p2;
  
  p=waitpid (p, status, options);
    
  if (pidtable)pidtable[(int)p][0]=pidtable[(int)p][1]=0;
  return p;
}
pid_t vwait (pid_t *p)
{
  pid_t p2;
  int rv;
  int handle_failure;
  
  if (atoigetenv("RETURN_ON_FAILURE"))handle_failure=RETURN_ON_FAILURE;
  else handle_failure=EXIT_ON_FAILURE;
    
  
  p2=wait (&rv);
  rv=check_process("forked::T-Coffee", p2, rv,handle_failure);
  if ( p) p[0]=rv;
  
  return p2;
}

#ifdef keepdoinstuff
int * pid2cpid (int pid)
{
  sprintf (tmp, "/var/tmp/%d.plist");
  
  fprintf (com, "ps -f > %s", tmp);
  fp=vfopen (tmp, "r");
  list=file2string(tmp);
  
  for (a=2; a< atoi (list[0]); a++)
    {
      max=MAX(atoi(list[a][1]), max);
      max=MAX(atoi(list[a][2]), max);
    }
  child= vcalloc ( max+2, sizeof (int));
  parent=vcalloc ( max+2, sizeof (int));
  child[max+1]=parent[max+1]=-1;
  
  for (a=2; a< atoi (list[0]); a++)
    {
      p=atoi(list[a][1]);
      c=atoi(list[a][2]);
      parent[p]=c;
    }
  child [pid]=1;
  while ( pid2cpid2(parent, child));
  child[pid]=0;
  return child;
}

int pid2cpid2 (int *parent, int *child)
{
  int a=0, n=0;
  while ( parent[a]!=-1)
    {
      
      if (parent[a] && child[a] && !child[parent[a]])
	{
	  child [parent[a]]=1;
	  n++;
	}
    }
  return n;
}
#endif
int get_child_list (int pid,int *clist);
int kill_child_list (int *list);
int kill_child_pid(int pid)
{
  int *list;
  int n,a, cpid;
  
  cpid=getpid();
  
  list=vcalloc (MAX_N_PID, sizeof (int));
  
  while ((n=get_child_list (pid,list)))
    {
      kill_child_list (list);
    }
  
  for (a=0; a<MAX_N_PID; a++)
    {
      if ( list [a] && a!=cpid)
	{
	  shift_lock (a,cpid, LERROR, LERROR, LSET);
	  shift_lock (a,cpid, LWARNING, LWARNING, LSET);
	  lock  (a, LLOCK, LRELEASE, " ");
	  lock  (a, LERROR,LRELEASE, " ");
	  lock  (a, LWARNING,LRELEASE, " ");
	}
    }
  return;
}

int kill_child_list (int *list)
{
  int a;
  int cpid=getpid();
  for (a=0; a<MAX_N_PID; a++)
    {
      if (list[a]==1 && a!=cpid)
	{
	  kill (a, SIGTERM);
	  list[a]=-1;
	}
    }
}
	  
int get_child_list (int pid,int *clist)
{
  char ***list;
  char *lockf;
  int a;
  int n=0;
  
  lockf=lock2name (pid, LLOCK);
  if ( lockf && file_exists (NULL,lockf)) 
    {
      list=file2list (lockf, "\n");
      a=1;
      while (list && list[a])
	{
	  n+=get_child_list (atoi(list[a++][1]), clist);
	}
      free_arrayN ((void **)list, 3);
    }
  
  if (!clist[pid]){clist[pid]=1; n++;}
  return n;
}


int kill_child_pid_pld()
{
  int n;
  string2file (logfile, "a","\n----TC: %d kill ALL CHILDREN", getpid());
  
  //  kill (0, SIGTERM); //send a sigterm to ALL children processes
  //return 1;
  
  if ( !pidtable)return 0;
  else
    {
      int a;
      pid_t cpid;
      cpid=getpid();
      for (a=0; a<pidtable_s; a++)
	{
	  if (pidtable[a][1] && pidtable[a][0]==cpid )
	    {
	      if (debug_lock)fprintf ( stderr, "\n--- KILL %d from TC (%d)\n", a, getpid());
	      
	      pidtable[a][1]=pidtable[a][0]=0;
	      kill((pid_t)a, SIGTERM);
	      //lock (a, LLOCK, LRELEASE, "");
	      shift_lock(a,getpid(),LERROR,LERROR, LSET);
	      shift_lock(a,getpid(),LWARNING,LWARNING, LSET);
	      
	      n++;
	    }
	}
    }
  return n;
}
	  



void unpack_all_perl_script (char *script)
{
  int a=0;
  FILE *fp;
  char command[1000];
  
  while ( !strm(PerlScriptName[a], "EndList"))
    {
      if (script==NULL || strm (script, "all") ||strm (script, "perl_script")|| strm (script,PerlScriptName[a]))
	{
	  fprintf ( stderr, "\nUnpack Script %s\n", PerlScriptName[a]);
	  fp=vfopen ( PerlScriptName[a], "w");
	  fprintf (fp, "%s\n%s\n", PERL_HEADER,PerlScriptFile[a]);
	  vfclose (fp);
	  sprintf ( command, "chmod u+x %s",PerlScriptName[a] );
	  safe_system (command);
	}
      a++;
    }
  myexit (EXIT_SUCCESS);
}
int satoi (char *);
int satoi (char *c)
{
  if ( !c)return 0;
  else return atoi (c);
}
  
int unpack_perl_script (char *name, char ***unpacked, int n)
{
  int a=0;
  
  set_file2remove_extension(".pl", SET);
  
  if ( name==NULL) unpack_all_perl_script (NULL);
  while ( !strm(PerlScriptName[a], "EndList") && !strm ( name, PerlScriptName[a]))
    {
      a++;
    }
  
  if ( strm(PerlScriptName[a], "EndList")|| (check_file_exists (name) && isexec(name) && satoi(getenv("RUN_LOCAL_SCRIPT"))) || (pg_is_installed (name) &&(satoi(getenv("RUN_LOCAL_SCRIPT"))) ))
       {
	 if ( check_file_exists (name) && ! strstr (name, "/") && isexec(name) && satoi(getenv ("RUN_LOCAL_SCRIPT")))
	   {
	     add_warning ( stderr, "Local Copy of %s detected [Debug mode]. DELETE this file", name);
	     sprintf ( unpacked[0][n], "%s", name);
	     sprintf ( unpacked[1][n], "./%s", name);
	     sprintf ( unpacked[2][n], "shell");
	   }
	 else if ( pg_is_installed (name) &&satoi(getenv("RUN_LOCAL_SCRIPT")) )
	   {
	     add_warning ( stderr, "Running external copy of %s [Debug mode].", name);
	     sprintf ( unpacked[0][n], "%s", name);
	     sprintf ( unpacked[1][n], "%s", name);
	     sprintf ( unpacked[2][n], "local");
	   }
	 else if ( strstr (name, "t_coffee"))
	   {
	     /*Cygwin pb: t_coffee cannot call itself: making a local copy for recursion*/
	     char buf[1000];
	     char *b2;
	     b2=vtmpnam (NULL);
	     sprintf (buf, "cp `which %s` %s; chmod u+x %s",name, b2, b2);
	     safe_system (buf);
	     sprintf ( unpacked[0][n], "%s", name);
	     sprintf ( unpacked[1][n], "%s", b2);
	     sprintf ( unpacked[2][n], "shell");
	     
	   }
	 else
	   {
	     sprintf ( unpacked[0][n], "%s", name);
	     sprintf ( unpacked[1][n], "%s", name);
	     sprintf ( unpacked[2][n], "shell");
	   }
       }
  else
    {
      FILE *fp;
     
      sprintf ( unpacked[0][n], "%s", name);
      sprintf ( unpacked[1][n], "%s", vtmpnam(NULL));
      sprintf ( unpacked[2][n], "unpack");
      fp=vfopen (unpacked[1][n], "w");
      fprintf (fp, "%s\n%s\n", PERL_HEADER,PerlScriptFile[a]);            
      vfclose (fp);
      printf_system_direct ("chmod u+x %s", unpacked[1][n]);
      
    }
  set_file2remove_extension(".pl", UNSET);
  return ++n;
}
	
/*********************************************************************/
/*                                                                   */
/*                         IO FUNCTIONS                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
char *program_name;
char *error_file;
void   set_command_line (char *cl)
{
  in_cl=vcalloc ( strlen (cl)+1, sizeof (char));
  sprintf (in_cl, "%s", cl);
}
FILE * print_command_line (FILE *fp )
{
  fprintf ( fp, "# Command Line: %s [PROGRAM:%s]\n",in_cl, PROGRAM);
  return fp;
}
int check_dir_getenv ( char *string);
char *get_plugins_4_tcoffee_old ( char *mode)
{
  static char *plugins_4_tcoffee;
  
  if ( !plugins_4_tcoffee)plugins_4_tcoffee=vcalloc ( 1000, sizeof (char));
  
  if (mode)plugins_4_tcoffee[0]='\0';

  
  if ( plugins_4_tcoffee[0])return plugins_4_tcoffee;
  else if ( mode) sprintf (plugins_4_tcoffee, "%s", mode);
  else if ( check_dir_getenv ("PLUGINS_4_TCOFFEE"))
    {
      sprintf (plugins_4_tcoffee, "%s", getenv ("PLUGINS_4_TCOFFEE"));
    }
  else
    {
      sprintf (plugins_4_tcoffee, "%s/plugins/%s/", get_dir_4_tcoffee(), get_os());
    }
  //if (!isdir (plugins_4_tcoffee)){printf_exit (EXIT_FAILURE, stderr, "ERROR: The declared plugin directory for T-Coffee is invalid: [%s] [FATAL:%s %s]", plugins_4_tcoffee, PROGRAM, VERSION); }
  return plugins_4_tcoffee;
}
char *get_home_4_tcoffee ()
{
  static char *home_4_tcoffee;
  static char home[1000];
  

  if ( !home_4_tcoffee)
    home_4_tcoffee=vcalloc ( 1000, sizeof (char));
  
  if ( home_4_tcoffee[0])return home_4_tcoffee;
  else if ( check_dir_getenv ("HOME_4_TCOFFEE"))
    {
      sprintf (home_4_tcoffee, "%s", getenv ("HOME_4_TCOFFEE"));
    }
  else if ( check_dir_getenv ("HOME"))
    {
      sprintf (home_4_tcoffee, "%s", getenv ("HOME"));
      sprintf (home, "%s", home_4_tcoffee);
    }
  else if ( check_dir_getenv ("TMP"))
    {
      sprintf (home_4_tcoffee, "%s", getenv ("TMP"));
    }
  else if (check_dir_getenv ("TEMP"))
    {
      sprintf (home_4_tcoffee, "%s", getenv ("TEMP"));
    }
  else 
    {
      printf_exit (EXIT_FAILURE, stderr, "ERROR: Could not set a HOME directory.\nSet any of the following environement variables to some suitable location: HOME, HOME_4_TCOFFEE, TMP or TEMP [FATAL:%s]\n", PROGRAM);
    }
  
 
  return home_4_tcoffee;
}
char *get_dir_4_tcoffee()
{
  static char dir_4_tcoffee[1000];
   
  if (dir_4_tcoffee[0])return dir_4_tcoffee;
  else
    {
      if ( getenv ("UNIQUE_DIR_4_TCOFFEE"))sprintf (dir_4_tcoffee, "%s", getenv("UNIQUE_DIR_4_TCOFFEE"));
      else if ( getenv ("DIR_4_TCOFFEE"))sprintf (dir_4_tcoffee, "%s", getenv("DIR_4_TCOFFEE"));
      else sprintf ( dir_4_tcoffee, "%s/.t_coffee",get_home_4_tcoffee());
      my_mkdir (dir_4_tcoffee);
    }
  return dir_4_tcoffee;
}
char *get_tmp_4_tcoffee ()
{
  static char tmp_4_tcoffee [1000];
  
  if ( tmp_4_tcoffee[0])return tmp_4_tcoffee;
  else
    {
      
      if (getenv ("UNIQUE_DIR_4_TCOFFEE"))sprintf (tmp_4_tcoffee, "%s", getenv("UNIQUE_DIR_4_TCOFFEE"));
      else if ( getenv ("TMP_4_TCOFFEE"))sprintf (tmp_4_tcoffee, "%s", getenv("TMP_4_TCOFFEE"));
      else
	{
	  char command [1000];
	  
	  if ( strm (get_os(), "windows"))
	    {
	      sprintf ( tmp_4_tcoffee, ".TCtmp");
	    }
	  else if (isdir("/var/tmp"))
	    sprintf (tmp_4_tcoffee, "/var/tmp");
	  else
	    {
	    sprintf ( tmp_4_tcoffee, "%s/tmp", get_dir_4_tcoffee());
	    }
	}
      

      my_mkdir (tmp_4_tcoffee);
    }

  return tmp_4_tcoffee;
}
char *get_cache_4_tcoffee ()
{

  static char cache_4_tcoffee [1000];
  if ( cache_4_tcoffee[0])return cache_4_tcoffee;
  else
    {
      if (getenv ("UNIQUE_DIR_4_TCOFFEE"))sprintf (cache_4_tcoffee, "%s", getenv("UNIQUE_DIR_4_TCOFFEE"));
      else if ( getenv ("CACHE_4_TCOFFEE"))sprintf (cache_4_tcoffee, "%s", getenv("CACHE_4_TCOFFEE"));
      else sprintf ( cache_4_tcoffee, "%s/cache/", get_dir_4_tcoffee());
      
      my_mkdir(cache_4_tcoffee); /*Do not use mkdir: not yet initialized*/
    }
  return cache_4_tcoffee;
}

char *get_mcoffee_4_tcoffee ()
{
  static char mcoffee_4_tcoffee [1000];
  if ( mcoffee_4_tcoffee[0])return mcoffee_4_tcoffee;
  else
    {
      if (getenv ("UNIQUE_DIR_4_TCOFFEE"))sprintf (mcoffee_4_tcoffee, "%s", getenv("UNIQUE_DIR_4_TCOFFEE"));
      else if ( getenv ("MCOFFEE_4_TCOFFEE"))sprintf (mcoffee_4_tcoffee, "%s", getenv("MCOFFEE_4_TCOFFEE"));
      else sprintf ( mcoffee_4_tcoffee, "%s/mcoffee/", get_dir_4_tcoffee());
      my_mkdir (mcoffee_4_tcoffee);
    }
  return mcoffee_4_tcoffee;
}
char *get_methods_4_tcoffee ()
{
  static char methods_4_tcoffee [1000];
  if ( methods_4_tcoffee[0])return methods_4_tcoffee;
  else
    {
      if (getenv ("UNIQUE_DIR_4_TCOFFEE"))sprintf (methods_4_tcoffee, "%s", getenv("UNIQUE_DIR_4_TCOFFEE"));
      else if ( getenv ("METHODS_4_TCOFFEE"))sprintf (methods_4_tcoffee, "%s", getenv("METHODS_4_TCOFFEE"));
      else sprintf ( methods_4_tcoffee, "%s/methods/", get_dir_4_tcoffee());
      my_mkdir (methods_4_tcoffee);
    }
  return methods_4_tcoffee;
}

char *get_plugins_4_tcoffee ()
{
  static char plugins_4_tcoffee [1000];
  if ( plugins_4_tcoffee[0])return plugins_4_tcoffee;
  else
    {
      if (getenv ("UNIQUE_DIR_4_TCOFFEE"))sprintf (plugins_4_tcoffee, "%s", getenv("UNIQUE_DIR_4_TCOFFEE"));
      else if (isdir4path (getenv("PLUGINS_4_TCOFFEE")))sprintf (plugins_4_tcoffee, "%s", getenv("PLUGINS_4_TCOFFEE"));
      else sprintf (plugins_4_tcoffee, "%s/plugins/%s", get_dir_4_tcoffee(), get_os());
      my_mkdir(plugins_4_tcoffee);
    }
  return plugins_4_tcoffee;
}
char *get_lockdir_4_tcoffee ()
{
  static char lockdir_4_tcoffee [1000];
  char *v;


  

  v=getenv ("LOCKDIR_4_TCOFFEE");
  
  if ( lockdir_4_tcoffee[0])return lockdir_4_tcoffee;
  else
    {
      
      if (getenv ("UNIQUE_DIR_4_TCOFFEE"))sprintf (lockdir_4_tcoffee, "%s", getenv("UNIQUE_DIR_4_TCOFFEE"));
      else if (v && strstr (v, "TMP"))sprintf (lockdir_4_tcoffee, "%s", getenv("TMP_4_TCOFFEE"));
      else if (v && strstr (v, "LOCAL"))sprintf (lockdir_4_tcoffee, "%s", getcwd(NULL,0));
      else if (v && strstr (v, "."))sprintf (lockdir_4_tcoffee, "%s", getcwd(NULL,0));
      else if (v)sprintf (lockdir_4_tcoffee, "%s", v);
      else if (isdir("/var/tmp"))sprintf (lockdir_4_tcoffee, "/var/tmp");
      else sprintf (lockdir_4_tcoffee, "%s", getcwd(NULL,0));
      
      my_mkdir(lockdir_4_tcoffee);
    }
  
  
  return lockdir_4_tcoffee;
}


int get_nproc ()
{
  static int nproc;

  if (nproc);
  else if ( get_int_variable ("n_core"))
    {
      nproc=get_int_variable ("n_core");
    }
  else if ( getenv ("NUMBER_OF_PROCESSORS_4_TCOFFEE"))
    {
      nproc=atoi(getenv ("NUMBER_OF_PROCESSORS_4_TCOFFEE"));
    }
  else if ( getenv ("NUMBER_OF_PROCESSORS"))
    {
      nproc=atoi(getenv ("NUMBER_OF_PROCESSORS"));
    }
  else if ( file_exists (NULL,"/proc/cpuinfo"))
    {
      char *tmp;
      char string[100];
      tmp=vtmpnam (NULL);
      printf_system_direct ("grep \"^processor\" /proc/cpuinfo | wc -l>%s", tmp);
      sprintf ( string, "%s", file2string (tmp));
      chomp (string);
      nproc=atoi (string);
    }
  else  
    nproc=1;
    
  return nproc;
}

  



char * get_os()
{
  static char os[100];
  char *file;
  
  if ( os[0])return os;
  else
    {
      char *s;

      file=tmpnam (NULL);
      printf_system_direct ("uname > %s", file);

      s=file2string (file);
      lower_string (s);
      
      if (strstr (s, "cygwin"))sprintf ( os, "windows");
      else if ( strstr (s, "linux"))sprintf ( os, "linux");
      else if ( strstr (s, "osx"))sprintf ( os, "macosx");
      else if ( strstr (s, "darwin"))sprintf ( os, "macosx");
      else sprintf (os, "linux");
      vfree (s);
      vremove (file);
    }
  return os;
}




  
int cputenv (char *string, ...) 
{
  

  size_t len ;
  char *env;
  char *file;
  char *s;
  char *s2;
  int r;
  

  if (!string)return 0;
  
  cvsprintf (s, string);
  
  s2=vcalloc (strlen (s)+1, sizeof (char));
  sprintf ( s2, "%s", s);
  
  r=putenv (s2);
  //vfree (s); //Potential leak
  return r;
}


int fcputenv   (char *file, char *mode,char * string, ...)
{
  va_list ap;
  FILE *fp;
  if (!string)return 0;
  
  if (!(fp=vfopen (file, mode)))return 0;
  
  va_start (ap, string);
  vfprintf (fp, string, ap);
  vfclose (fp);
  va_end (ap);
  return 1;
}
int isdir4path (char *p)
{
   if ( !p) return 0;
   if ( !p || access (p, F_OK)==-1 || access (p, W_OK)==-1 || access(p, R_OK)==-1 || access (p, X_OK)==-1)return 0;
   
   return 1;
}
int check_dir_getenv ( char *string)
{
  char *p;
  return (isdir4path(p=getenv ( string)));

}




int  set_unique_dir_4_tcoffee (char *dir);
int  set_unique_dir_4_tcoffee (char *dir)
{
  static char **string;
  int m, n;
  m=n=0;
  if ( !dir || !isdir(dir) || strm (dir, "no"))return 0;
  string=declare_char (10, 100);
  sprintf ( string[m++], "DIR_4_TCOFFEE=%s", dir);putenv (string[n++]);
  sprintf ( string[m++], "CACHE_4_TCOFFEE=%s", dir);putenv (string[n++]);
  sprintf ( string[m++], "TMP_4_TCOFFEE=%s", dir);putenv (string[n++]);
  sprintf ( string[m++], "PLUGINS_4_TCOFFEE=%s", dir);putenv (string[n++]);
  sprintf ( string[m++], "MCOFFEE_4_TCOFFEE=%s", dir);putenv (string[n++]);
  sprintf ( string[m++], "METHODS_4_TCOFFEE=%s", dir);putenv (string[n++]);
  
  return 1;
}



  

void myexit (int signal)
{
    
  if (clean_exit_started==1)return; //protects processes while they are doing a clean exit
  global_exit_signal=signal;
  exit (global_exit_signal); //ONLY BARE EXIT!!!!!!!!!!!!!!
}


static int n_warning;
static char **warning_list;

FILE *fatal_exit (FILE *fp,int exit_mode, char *string, ...)
{
  va_list ap;
  va_start (ap, string);
  vfprintf (fp, string, ap);
  va_end (ap);
  myexit (exit_mode);
  return fp;
}
static int warning_mode;
int set_warning_mode ( int mode)
{
  warning_mode=mode;
  return mode;
}

int  fprintf_error( FILE *fp, char *string, ...)
{
  char *msg;
  
  cvsprintf (msg, string);
  msg=substitute ( msg, "\n", "");
  msg=substitute ( msg, "ERROR", " ");
  if (fp)fprintf ( fp, "\n--ERROR: %s\n", msg);
  if ( clean_exit_started) return EXIT_FAILURE;
  
  lock (getpid(), LERROR,LSET,"%d -- ERROR: %s\n",getpid(),msg);
  lock (getpid(), LERROR,LSET,"%d -- COM: %s\n",getpid(),in_cl);
  lock (getpid(), LERROR, LSET,"%d -- STACK: %d -> %d\n",getpid(), getppid(),getpid());
  
  vfree (msg);
  return EXIT_FAILURE;
}
void printf_exit  (int exit_code, FILE *fp, char *string, ...)
{
  char *msg;
  
  cvsprintf (msg, string);
  myexit(fprintf_error (fp, string));
  myexit (exit_code);
}


FILE *add_warning (FILE *fp, char *string, ...)
{
  char *buf;
    
  if ( warning_mode==NO || getenv("NO_WARNING_4_TCOFFEE"))return fp;
  else
    {
      
      cvsprintf (buf, string);
      if (fp)fprintf (fp, "\npid %d -- %s\n",getpid(), buf);
      if ( clean_exit_started)return fp;
      lock(getpid(),LWARNING, LSET, "%d -- WARNING: %s\n", getpid(),buf);
      
      vfree (buf);
    }
  return fp;
}

    
 
int   count_n_res_in_array  (char *array, int len)
      {
	return count_n_symbol_in_array(array, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", len);
      }
int   count_n_gap_in_array  (char *array, int len)
      {
	int l;
	if ( len<=0 ||len>strlen(array) )l=strlen(array);
	else l=len;
	
	return l- count_n_res_in_array (array,len);
      }
int count_n_symbol_in_array ( char *array, char *array_list, int len)
      {
	int a=0, t=0;
	int l;
	
	if ( len<=0 ||len>strlen(array) )l=strlen(array);
	else l=len;

	for ( a=0; a< l; a++)t+=is_in_set (array[a], array_list);
	return t;
      }

char* count_strings_in_file ( char *in, char *out)
{
  FILE *fp;
  int n,c;
  char **list, **result;

  
  if (!out) out=vtmpnam (NULL);
  list=declare_char (count_n_line_in_file(in)+1, measure_longest_line_in_file (in)+1);
  
  n=0;
  fp=vfopen (in, "r");
  while ((c=fgetc (fp))!=EOF)
    {
      ungetc (c, fp);
      fscanf (fp, "%s\n",list[n]);
      n++;
    }
  vfclose (fp);
  
  result=count_strings (list, n);
  
  
  n=0;
  fp=vfopen (out, "w");
  while (result[n])fprintf ( fp,"%s\n", result[n++]);
  vfclose (fp);
  
  free_char (list, -1);
  free_char (result, -1);
  
  return out;
}

int ** count_int_strings (int **array, int len, int s)
{
  int **result;
  int a,n;
  
  sort_list_int (array,s, s,0, len-1);
  result=vcalloc (len, sizeof (int*));
  for (n=-1,a=0; a<len; a++)
    {
      if (n==-1 || memcmp (array[a], result[n], sizeof (int)*s)!=0)
	{
	  n++;
	  result[n]=vcalloc (s+1, sizeof (int));
	  memcpy (result[n],array[a], sizeof (int)*s);
	  fprintf ( stdout, "*");
	}
      result[n][s]++;
    }
  sort_int (result, s+1, s, 0, n);
  result=vrealloc ( result, sizeof (int*)*n);
  return result;
}
  
char** count_strings ( char **array, int len)
{
  int a, c, n;
  char *cs;
  char **result;
  
  result=vcalloc (len+1, sizeof (char*));
  array=sort_string_array (array, len);

  for (c=0, a=0, n=0, cs=NULL; a< len; a++)
    {
      if (cs == NULL || !strm (cs, array[a]))
	{
	  if (cs)
	    {
	      result[c]=vcalloc (strlen (cs)+20, sizeof (char));
	      sprintf ( result[c], "%s %d", cs, n);
	      c++;
	    }
	  
	  n=1;
	  cs=array[a];
	}
      else n++;
    }
  
  result[c]=vcalloc (strlen (cs)+20, sizeof (char));
  sprintf ( result[c], "%s %d", cs, n);
 
  return result;
}
	  
int   get_first_non_white_char (char *name)
{
  if ( !name) return 0;
  else if ( !file_exists (NULL,name))return 0;
  else
    {
      FILE *fp;
      char c;
      
      fp=vfopen (name, "r");
      while (isspace ((c=fgetc (fp)))&& c!=EOF);
      vfclose (fp);
      return c;
    }
}
int   count_n_char_x_in_file(char *name, char x)
      {
      FILE *fp;
      int n, c;

      n=0;
      fp=vfopen(name, "r");
      while ( (c=fgetc(fp))!=EOF)n+=(c==x);
      vfclose (fp);
      return n;
      }
int   count_n_char_in_file(char *name)
      {
      int  c, n;
      FILE *fp;

      n=0;
      fp=vfopen(name, "r");
      while ( (c=fgetc(fp))!=EOF){n++;}
      vfclose (fp);
      return n;
      }
int count_n_line_in_file ( char *name )
     {
      int  c, n;
      FILE *fp;

      n=0;
      fp=vfopen(name, "r");
      while ( (c=fgetc(fp))!=EOF)n+=(c=='\n');
      vfclose (fp);
      return n;
      }
int measure_longest_line_in_file ( char *name )
     {
      int  c;
      FILE *fp;
      int longest=0;
      int current=0;


      fp=vfopen(name, "r");
      while ( (c=fgetc(fp))!=EOF)
	  {
	      if ( c=='\n'){longest=MAX(longest, current+1);current=0;}
	      else current++;
	  }
      longest=MAX(longest, current+1);

      vfclose (fp);
      return longest;
      }
char *input_name ()
	{
	char *string;
	int a;
	char ch;
	
	string= vcalloc ( 500, sizeof ( char));
	
	a=0;
    	while ( ( ch=getchar())!='\n' && a<500)
    			string[a++]=ch;
    	string[a]='\0'; 
    	
    	if ( string[0]=='\0')
    		{
    		vfree (string);
    		return NULL;
    		}
    	else
    		return string;
    	}

FILE * vtmpfile()
    {
        return tmpfile();
    }
int string_variable_isset (char *name)
{
  if (store_string_variable (name,NULL, ISSET)==NULL)return 0;
  else return 1;
}
char* set_string_variable (char *name, char * v)
{
  return store_string_variable (name, v, SET);
}
char * get_string_variable (char *name)
{
  return store_string_variable (name,NULL, GET);
}
char* unset_string_variable (char *name)
{
  return store_string_variable (name,0, UNSET);
}

char* store_string_variable (char *name, char* v, int mode)
{
  static char **name_array, **val_array;
  static int n;
  int a;
  
  if ( mode == SET)
    {
      
      for (a=0; a<n; a++)
	{
	  if ( strm (name,name_array[a]))
	    {
	      if (v)
		{
		  val_array[a]=vrealloc (val_array[a], strlen (v)+1);
		  sprintf (val_array[a],"%s",v);
		}
	      else val_array[a]='\0';
	      return v;
	    }
	}
      if (!name_array)
	{
	  name_array=vcalloc (1, sizeof (char*));
	  val_array=vcalloc  (1, sizeof (char*));
	}
      else
	{
	  name_array=vrealloc (name_array, (n+1)*sizeof (char*));
	  val_array=vrealloc (val_array, (n+1)*sizeof (char*));
	}
      name_array[n]=vcalloc ( strlen (name)+1, sizeof (char));
      val_array[n]=vcalloc ( strlen (v)+1, sizeof (char));
      
      sprintf ( name_array[n], "%s", name);
      sprintf ( val_array[n], "%s", v);
      n++;
      
      return v;
      
    }
  else if ( mode == ISSET)
    {
       for (a=0; a<n; a++) 
	 if ( strm (name_array[a], name))return (char *)1;
    }
  else if ( mode == UNSET)
    {
      for (a=0; a<n; a++) 
	if ( strm (name_array[a], name))
	  {
	    name_array[a][0]='\0';
	    val_array[a][0]='\0';
	    return 0;
	  }
      add_warning (stdout, "Could not UNSET the value of %s. You must SET the value before it is used", name);
    }
  else if (mode==GET)
    {
      for (a=0; a<n; a++) 
	{
	  if ( strm (name_array[a], name))
	    return val_array[a];
	}
    }
  return NULL;
}
int int_variable_isset (char *name)
{
  return store_int_variable (name,0, ISSET);
}
int set_int_variable (char *name, int v)
{
  return store_int_variable (name, v, SET);
}
int get_int_variable (char *name)
{
  return store_int_variable (name, 0, GET);
}
int unset_int_variable (char *name)
{
  return store_int_variable (name,0, UNSET);
}

int store_int_variable (char *name, int v, int mode)
{
  static char **name_array; 
  static int *val_array;
  static int n;
  int a;
  
  if ( mode == SET)
    {
      for (a=0; a<n; a++)
	{
	  if ( strm (name,name_array[a]))
	    {
	      val_array[a]=v;
	      return v;
	    }
	}
      if (!name_array)
	{
	  name_array=vcalloc (1, sizeof (char*));
	  val_array=vcalloc  (1, sizeof (int));
	}
      else
	{
	  name_array=vrealloc (name_array, (n+1)*sizeof (char*));
	  val_array=vrealloc (val_array, (n+1)*sizeof (int));
	}
      name_array[n]=vcalloc ( strlen (name)+1, sizeof (char));
      sprintf ( name_array[n], "%s", name);
      val_array[n]=v;
      n++;
      
      return v;
      
    }
  else if ( mode == ISSET)
    {
       for (a=0; a<n; a++) 
	 if ( strm (name_array[a], name))return 1;
    }
  else if ( mode == UNSET)
    {
      for (a=0; a<n; a++) 
	if ( strm (name_array[a], name))
	  {
	    name_array[a][0]='\0';
	    val_array[a]=0;
	    return 0;
	  }
      add_warning (stdout, "Could not UNSET the value of %s. You must SET the value before it is used", name);
    }
  else if (mode==GET)
    {
      for (a=0; a<n; a++) 
	{
	  if ( strm (name_array[a], name))
	    return val_array[a];
	}
    }
  return 0;
}

      
char * get_proxy_from_env ()
{
  char *proxy=NULL;
  
  if ((proxy=get_string_variable ("cl_proxy"))){;}//Command line proxy always wins
  else if ((proxy=getenv ("http_proxy_4_TCOFFEE"))); 
  else if ((proxy=get_string_variable ("proxy")));//use default T-Coffee proxy
  else if ( getenv ("HTTP_proxy") && getenv ("http_proxy")){return getenv ("HTTP_proxy");}//get environement proxy
  else if ((proxy=getenv ("HTTP_proxy")));//id 
  else if ((proxy=getenv ("http_proxy")));//id
  else if ((proxy=getenv ("HTTP_PROXY")));//id
  else if ((proxy=getenv ("ALL_proxy")));//id
  else if ((proxy=getenv ("all_proxy")));//id
  else if ((proxy=getenv ("ALL_PROXY")));//id
  
  if (proxy)set_proxy(proxy);
  else
    {proxy=vcalloc (1, sizeof(char));}
      
  return proxy;
}
char *get_proxy ()
{
  return get_proxy_from_env ();
}
int set_proxy (char *proxy)
{
  char *http;
  if (!proxy) return 0;
  
  cputenv ("HTTP_proxy_4_TCOFFEE=%s", proxy);
  cputenv ("HTTP_proxy=%s", proxy);
  cputenv ("http_proxy=%s", proxy);
  cputenv ("HTTP_PROXY=%s", proxy);
  cputenv ("ALL_proxy=%s", proxy);
  cputenv ("ALL_PROXY=%s", proxy);
  cputenv ("all_proxy=%s", proxy);
  
  return 1;
}

FILE* warning_msg(FILE*fp)
{
  char *msg;

  msg=lock ( getpid(),LWARNING, LREAD,NULL);
  if (!msg) return fp;
  fprintf ( fp, "\n\n");
  fprintf ( fp, "*************************************************************************************************\n");
  fprintf ( fp, "*                        WARNING LIST                                    \n");
  fprintf ( fp, "%s",msg);
  fprintf ( fp, "*************************************************************************************************\n");
  return fp;
}
FILE* stack_msg(FILE*fp)
{
  char *error;

  error=lock ( getpid(),LERROR, LREAD,NULL);
  fprintf ( fp, "\n\n");
  fprintf ( fp, "*************************************************************************************************\n");
  fprintf ( fp, "*                        FULL TRACE BACK PID: %d                                    \n", getpid());
  fprintf ( fp, "%s", error);
  fprintf ( fp, "*************************************************************************************************\n");
  return fp;
}
FILE * install_msg(FILE *fp)
{
  fprintf ( fp, "\n\n");
  fprintf ( fp, "*************************************************************************************************\n");
  fprintf ( fp, "*                        CONFIGURATION: Missing Package                                     \n");
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "* It looks like a package is either not installed or not on your PATH\n (see trace)\n"); 
  fprintf ( fp, "* If it is on your path, declare its location:\n");
  fprintf ( fp, "*                  export PATH=<executable directory>:$PATH\n");
  fprintf ( fp, "* Make this permanent by adding this line to the file\n");
  fprintf ( fp, "*                  ~/.bashrc\n");
  fprintf ( fp, "* If this package is not installed but supported you can try to install it via t_coffee:\n");
  fprintf ( fp, "*                  t_coffee -other_pg install <pakage name>\n");
  fprintf ( fp, "* Otherwise you must install it yourself\n");
  fprintf ( fp, "*************************************************************************************************\n");
  
  return fp;
}
FILE* proxy_msg(FILE*fp)
  {
  fprintf ( fp, "\n\n");
  fprintf ( fp, "*************************************************************************************************\n");
  fprintf ( fp, "*                        CONFIGURATION: Faukty Network OR Missing Proxy                                        \n");
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "* It looks like you cannot access the network\n");    
  fprintf ( fp, "* Check that your network is up and running...\n");    
  fprintf ( fp, "* If you are behind a firewall, you must enter your proxy address to use webservices\n");
  fprintf ( fp, "* This address is usualy something like: http://some.place.here:8080\n");           
  fprintf ( fp, "* Todo this via the command line:\n");
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "*                  -proxy=<youremail>  \n");
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "*To make it permanent:\n");
  fprintf ( fp, "*                   export PROXY_4_TCOFFEE=<your email>\n");
  fprintf ( fp, "*Add this line to either:\n");
  fprintf ( fp, "*             <yourhome>/.bashrc\n");
  fprintf ( fp, "*         OR  %s/.t_coffee_env\n", get_dir_4_tcoffee());
  fprintf ( fp, "*************************************************************************************************\n");
  
  return;
}
FILE* email_msg(FILE*fp)
{
  fprintf ( fp, "\n\n");
  fprintf ( fp, "*************************************************************************************************\n");
  fprintf ( fp, "*                        CONFIGURATION: Missing Email                                        \n");
  fprintf ( fp, "*                                                                                               \n");
  
  fprintf ( fp, "* This mode of T-Coffee uses the  EBI BLAST webservices. The EBI requires a valid E-mail     \n");
  fprintf ( fp, "* address for this service to be used (check: www.ebi.ac.uk/Tools/webservices/).                \n");            
  
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "* To provide the email, add to the command line:*\n");
  fprintf ( fp, "*                                                                                               \n");
  fprintf ( fp, "*                  -email=<youremail>  *\n");
  fprintf ( fp, "*To make it permanent:\n");
  fprintf ( fp, "*                   export EMAIL_4_TCOFFEE=<your email>\n");
  fprintf ( fp, "*Add this line to either:\n");
  fprintf ( fp, "*             ~/.bashrc\n");
  fprintf ( fp, "*         OR  %s/.t_coffee_env\n", get_dir_4_tcoffee());
  fprintf ( fp, "*************************************************************************************************\n");
  
  return fp;
}	    

void update_error_dir()
{
  char fdir[1000];
  char *dir, *s;
  char **list;
  int a=0;
  
  
}

void dump_error_file()
{
  char target[1000];
  char fdir[1000];
  char error[1000];
  char warning[1000];
  char **list, *s;
  int a=0;
  FILE *fp;
  char command[1000];
  
  sprintf ( target, "%s",getenv("ERRORFILE_4_TCOFFEE"));
  if ((fp=fopen (target, "w")))
    {
      fprintf ( fp, "\n######### RUN_REPORT      START  ######");
      fprintf ( fp, "\n######### PROGRAM_VERSION START  ######"); 
      fprintf ( fp, "\n          %s, %s", PROGRAM, VERSION);
      fprintf ( fp, "\n######### PROGRAM_VERSION END    ######");
      
      fprintf ( fp, "\n######### COMMAND_LINE    START  ######");
      fprintf ( fp, "\n%s", in_cl);
      fprintf ( fp, "\n######### COMMAND_LINE    END    ######\n"); 
      fprintf ( fp, "\n######### MESSAGES START    ######\n"); 
      
      stack_msg(fp);
      warning_msg (fp);
      
      fprintf ( fp, "\n######### MESSAGES END    ######\n"); 
      fprintf ( fp, "\n######### FILES START     ######\n"); 
      
      list=string2list (in_cl);
      for (a=1; a<atoi (list[0]); a++)
	{
	  FILE *fp2;
	  char c;
	  s=list[a];
	  if (file_exists (NULL,s))
	    {
	      fp2=fopen (s, "r");
	      fprintf ( fp, "\n************* Start_File: %s  *************\n", s);
	      while ((c=fgetc(fp2))!=EOF)fprintf ( fp, "%c", c);
	      fclose (fp2);
	      fprintf ( fp, "\n************* End_File: %s  *************", s);
	      
	    }
	}
      fprintf ( fp, "\n######### FILES END  ######\n"); 
      fprintf ( fp, "\n######### ENVIRONEMENT  ######\n"); 
      fclose (fp);
      printf_system_direct( command, "printenv >> %s", target);
            
      fprintf ( stderr, "\n#----- Dumped ErrorFile: %s\n",target);
    }
  else fprintf ( stderr, "\n#----- Could NOT Dumpe ErrorFile: %s -- Sorry \n", target);

}

   

FILE* error_msg(FILE*fp )
     {
       if ( no_error_report)return;

       fprintf( fp,"\n\t******************************************************************");
       fprintf( fp, "\n\t* Abnormal Termination");
       fprintf( fp, "\n\t* Job NOT Completed:[%s, %s]",PROGRAM, VERSION);
       fprintf( fp, "\n\t* Please CHECK:                                       ");
       fprintf( fp, "\n\t* \t-1 The format of your Input Files                 ");
       fprintf( fp, "\n\t* \t-2 The parameters                                 ");
       fprintf( fp, "\n\t* \t-3 The use of special characters in sequence names:");
       fprintf( fp, "\n\t* \t\t (@, |, %%...)");
       
       fprintf( fp, "\n\t* \t-4 The Online Doc (%s)                   ", URL);
       fprintf( fp, "\n\t* \t-5 Send the file:");
       fprintf (fp, "\n\t*");
       fprintf (fp, "\n\t*\t    %s ", getenv("ERRORFILE_4_TCOFFEE"));
       fprintf (fp,  "\n\t* to:");
       fprintf( fp, "\n\t* \t\t%s",EMAIL);
       
       fprintf( fp, "\n\t* If you run T-Coffee over the WEB:");
       fprintf( fp, "\n\t* \tWindows Cut and Paste is sometimes erratic and");
       fprintf( fp, "\n\t* \tit can loose carriage returns. If you suspect this,");
       fprintf( fp, "\n\t* \ttry to cut and paste through an intermediate application");
       fprintf( fp, "\n\t* \t(word pad) and inspect the results\n\n");
       fprintf( fp, "\n\t* CONFIDENTIALITY:");
       fprintf( fp, "\n\t* \tThe File %s may contain your personnal DATA", getenv("ERRORFILE_4_TCOFFEE"));
       fprintf( fp, "\n\t* \tRemove ALL confidential DATA from this file BEFORE sending it");
       fprintf( fp, "\n\t******************************************************************\n");
       print_command_line(fp);
       return fp;
     }
      

char *get_email_from_env ()
{
  char *email=NULL;
  if ( (email=get_string_variable ("cl_email")));
  else if ( (email=get_string_variable ("email")));
  else if ( (email=getenv ("EMAIL_4_TCOFFEE")));
  else if ( (email=getenv ("EMAIL")));
  else email=vcalloc ( 1, sizeof (char));
  return email;
}

int set_email (char *email)
{
  if (!email) return 0;
  
  cputenv ("EMAIL_4_TCOFFEE=%s", email);
  cputenv ("EMAIL=%s",email);
  
  return 1;
}
char *chomp (char *name)
{
  int a=0;
  while ( name[a]!='\n' && name[a]!='\0')a++;
  name[a]='\0';
  return name;
}
static Tmpname *tmpname;
static Tmpname *ntmpname;

static int n_tmpname;
static int file2remove_flag;

char *set_file2remove_extension (char *extension, int mode)
{
  static char ext[100];
  if (mode==SET)sprintf (ext, "%s", extension);
  else if ( mode==UNSET) ext[0]='\0';
  else if ( mode==GET);
  return ext;
}
int flag_file2remove_is_on ()
{ 
  return file2remove_flag;
}
void set_file2remove_on()
{
  file2remove_flag=1;
}
void set_file2remove_off()
{
  file2remove_flag=0;
}

char *add2file2remove_list (char *name)
{

  
  if ( !tmpname || !name)ntmpname=tmpname=vcalloc ( 1, sizeof (Tmpname));
  else if (!ntmpname->name);
  else ntmpname=ntmpname->next=vcalloc ( 1, sizeof (Tmpname));
  
  if (!name) return NULL;
  
  ntmpname->name=vcalloc(strlen(name)+1, sizeof (char));
  
  sprintf (ntmpname->name, "%s", name);
  return ntmpname->name;
}
//char *short_tmpnam_2(char *s);//used to generate very compact tmp names
void  initiate_vtmpnam (char *file)
{
  add2file2remove_list (NULL);
  tmpnam_2(NULL);
}
char *vtmpnam ( char *s1)
{
  char *s,*s2;

  n_tmpname++;
  
  standard_initialisation(NULL, NULL);
  
  s=vcalloc ( VERY_LONG_STRING, sizeof (char));
  s[0]='\0';
  
  s=tmpnam_2 (s);
  
  s2=add2file2remove_list (s);
  if (s!=s2)vfree (s);
  if (s1){sprintf (s1, "%s",s2);return s1;}
  else return s2;
}
     


int get_vtmpnam2_root()
{
  int MAX_TMPNAM_ROOT=10000;
  static int v;

  if (v) ;
  else
    {
      vsrand(0);
      v=rand()%MAX_TMPNAM_ROOT;
    }
  return v;
}
char *tmpnam_2 (char *s)
{ 
	static int root;   
	static int file;
	char buf[VERY_LONG_STRING];
	static char root2[VERY_LONG_STRING]; 
	static char *tmpdir;
	static int name_size;
	
	if ( !root || !s)
	{
		char *vtmpnam_prefixe;
		
		name_size=MAX( 2*L_tmpnam, MAXNAMES*2)+1;
		root=get_vtmpnam2_root();
		sprintf ( root2, "%d%d_", root, (int)getpid());
		
		vtmpnam_prefixe=vcalloc (strlen (root2)+strlen (get_tmp_4_tcoffee())+2, sizeof (char));
		sprintf (vtmpnam_prefixe, "%s/%s", get_tmp_4_tcoffee(), root2);
		set_string_variable ("vtmpnam_prefixe1", vtmpnam_prefixe);
		set_string_variable ("vtmpnam_prefixe2", root2);
		vfree (vtmpnam_prefixe);
	}
	
	if (!s)return NULL;
	tmpdir=get_tmp_4_tcoffee();
					
	sprintf (buf, "%s/%s%d_TCtmp%s",tmpdir,root2, file++,set_file2remove_extension (NULL, GET));
	if ( strlen(buf)>=name_size)s=vrealloc (s,(strlen(buf)+1)*sizeof (char));
	sprintf (s, "%s", buf);
	return s;
}
char *short_tmpnam_2(char *s)
{
  static int root;   
  static int file;
  char buf[VERY_LONG_STRING];
  static char root2[VERY_LONG_STRING]; 
  static char *tmpdir;
  static int name_size;
  
  if ( !root || !s)
    {
      char *vtmpnam_prefixe;
      
      name_size=MAX( 2*L_tmpnam, MAXNAMES*2)+1;
      root=get_vtmpnam2_root();
      sprintf ( root2, "%d%d", root,getpid());
      
      vtmpnam_prefixe=vcalloc (strlen (root2)+strlen (get_tmp_4_tcoffee())+2, sizeof (char));
      sprintf (vtmpnam_prefixe, "%s", root2);
      set_string_variable ("vtmpnam_prefixe1", vtmpnam_prefixe);
      set_string_variable ("vtmpnam_prefixe2", root2);
      vfree (vtmpnam_prefixe);
    }
  if (!s) return NULL;
  
  sprintf (buf, "%s%d%s",root2, file++,set_file2remove_extension (NULL, GET));
  if ( strlen(buf)>=name_size)s=vrealloc (s,(strlen(buf)+1)*sizeof (char));
  sprintf (s, "%s", buf);
  return s;
}

char *vremove2 (char *s)
{
  char list_file[1000];
  char ***list;
  int a;

	
  //Remove filenames with a wildcard
  
  sprintf (list_file, "list_file_%d", (int)getpid());
  printf_system_direct("ls -1 %s>%s 2>/dev/null", s, list_file);
  list=file2list (list_file, " ");
 
  a=0;
  while (list && list[a])
    {
      if ( file_exists (NULL,list[a][1]))
	{
	  vremove (list[a][1]);
	}
      a++;
    }
  vremove (list_file);
  return NULL;
}
void safe_remove (char *s)//remove even if the file is partly unaccessible
{
  FILE *fp;


  if ( !s) return;
  else if (!(fp=fopen (s, "w")))return;
  else 
    {
      fclose (fp);
      remove (s);
    }
}
    
char *vremove (char *s)
{

  
  if ( s && strstr (s, "*"))return vremove2(s);
  else if ( !s || !file_exists(NULL,s) ) return NULL;
  else if ( isdir (s))
    {
      rmdir (s);
      return NULL;
    }
  else
    {
      remove (s);
      return NULL;
    }
  return NULL;
}
int log_function ( char *fname)
{
  char command[1000];
  
  if ( file_exists (NULL,error_file))
    {
      
      printf_system_direct ("cp %s %s", error_file, fname);

      fprintf( stderr,"\n\t******************************************************************");
      fprintf( stderr, "\n\t* Full Log of [%s, %s] in File [%s]",PROGRAM, VERSION, fname);
      fprintf( stderr, "\n\t******************************************************************\n");
    }
  return 1;
}



FILE *NFP;/*Null file pointer: should only be open once*/

/*********************************************************************/
/*                                                                   */
/*                         CACHE_FUNCTION                            */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
static char *cache;
char * prepare_cache ( const char *mode)
{
  cache =vcalloc ( 10000, sizeof(char));
  
  if (strm (mode, "use"))
    {
      sprintf (cache, "%s",get_cache_4_tcoffee());
    }
  
  else if ( strm (mode, "ignore") ||  strm (mode, "no"))
    {
      
      cache=vtmpnam(cache);
      strcat (cache, "/");
      printf_system_direct ("mkdir %s",cache);
      
    }
  else if ( strm (mode, "update"))
    {
      cache=vtmpnam(cache);
      strcat (cache, "/");
      printf_system_direct ("mkdir %s",cache);
    }
  else if ( strm (mode, "local"))
    {
      cache[0]='\0';
    }
  else
    {
      sprintf ( cache, "%s/",mode);
      my_mkdir ( cache);
    }
  return cache;
	    
}

char * get_cache_dir()
{
  if ( cache==NULL){cache=vcalloc (1, sizeof (char));cache[0]='\0';}
  return cache;
}

void update_cache ()
{
  char old_cache[1000];

  sprintf ( old_cache, "%s", get_cache_dir());
  prepare_cache( "use");
  printf_system_direct ("mv %s* %s",old_cache, get_cache_dir());
  printf_system_direct ("rmdir %s",old_cache);
}
void ignore_cache()
{
  if (getenv4debug ("DEBUG_TMP_FILE"))
    {
      fprintf ( stderr, "\n[DEBUG_TMP_FILE:%s] TEMPORARY CACHE HAS NOT Been Removed:\n\t%s\n", PROGRAM,get_cache_dir());
    }
  else
    {

      printf_system_direct ("rm -r %s",get_cache_dir());
    }
  return;
  
}

FILE *fopenN   ( char *fname, char *mode, int max_n_tries, int delay);
FILE * vfopen  ( char *name_in, char *mode)
    {
    FILE *fp;
    int get_new_name;
    int tolerate_mistake;    
    int cache_used=0;
    FILE *tmp_fp;
    int c;
    static char *name;
    static char *name2;
    static char *stdin_file;

  
    if ( !name_in)return NULL;
    if (!name){name=vcalloc (1000, sizeof (char));}
    if (!name2){name2=vcalloc (1000, sizeof (char));}
    
    sprintf ( name, "%s", name_in);
    tild_substitute (name, "~", get_home_4_tcoffee());

    get_new_name=tolerate_mistake=0;    
    if ( mode[0]=='g'){get_new_name=1; mode++;}
    else if ( mode[0]=='t'){tolerate_mistake=1;mode++;}
/*Use the cached version from CACHE_4_TCOFFEE*/    
    else if ( mode[0]=='c'){cache_used=1;mode++;}

    if (name==NULL ||strm5 ( name, "no","NO","No","NULL","/dev/null") || strm2 (name, "no_file", "NO_FILE"))
    		{       
 		if ( NFP==NULL)NFP=fopen (NULL_DEVICE, mode);
 		return NFP;
 		} 
    else if ( strm3 (name,"stderr","STDERR","Stderr"))return stderr; 
    else if ( strm3 (name,"stdout","STDOUT","Stdout"))return stdout;
    else if ( strm3 ( name, "stdin","STDIN","Stdin"))
	{
	  if (!stdin_file)
	    {
	      stdin_file=vtmpnam (NULL);
	      tmp_fp=vfopen ( stdin_file, "w");
	      while ( (c=fgetc(stdin))!=EOF)fprintf (tmp_fp, "%c", c);
	      vfclose ( tmp_fp);
	    }
	  return vfopen (stdin_file, "r");
	}
	
    else if ( strm (name, "") && (strm (mode, "w") ||strm (mode, "a")) )return stdout;
    else if ( strm (name, "") && strm (mode, "r"))return stdin;    
    else if ( (fp= fopen ( name, mode))==NULL)
    		{
		if ( strcmp (mode, "r")==0 && cache_used==0)
		  {
		    sprintf ( name2, "%s%s",get_cache_dir(), name);		    
		    return vfopen ( name2, "cr");
		  }
    		else if ( strcmp (mode, "r")==0 && cache_used==1)
    			{
    			fprintf (stderr, "\n--COULD NOT READ %s\n", name);
    			if ( get_new_name){fprintf ( stderr, "\nNew name: ");return vfopen (input_name(), mode-1);}
    			else if ( tolerate_mistake)return NULL;
			else 
			    {	
			      myexit(fprintf_error (stderr, "\nFORCED EXIT (NON INTERACTIVE MODE pid %d)\n", getpid()));
			    }
			}
    		else if ( strcmp (mode, "a")==0 && cache_used==0)
		  {
		    sprintf ( name2, "%s%s",get_cache_dir(), name);		    
		    return vfopen ( name, "ca");
		  }
		else if ( strcmp (mode, "a")==0 && cache_used==1)
    			{
    			fprintf (stderr, "\nCOULD NOT Append anything to %s\n", name);
    			if ( get_new_name){fprintf ( stderr, "\nNew name: ");return vfopen (input_name(), mode-1);}
    			else if ( tolerate_mistake)return NULL;
			else 
			    {	
			      myexit(fprintf_error (stderr, "\nFORCED EXIT (NON INTERACTIVE MODE pid %d)\n", getpid()));
			    }
			}
		else if ( strcmp (mode, "w")==0)
    			{
    			fprintf (stderr, "\nCANNOT WRITE %s\n", name);
    			if ( get_new_name==1){fprintf ( stderr, "\nNew name: ");return vfopen (input_name(), mode-1);}	
    			else if ( tolerate_mistake)return NULL;
			else 
			    {
			     myexit(fprintf_error (stderr, "\nFORCED EXIT (NON INTERACTIVE MODE pid %d): %s %s\n", getpid(),(strcmp ( mode, "r")==0)?"READ":"WRITE", name));
			    
			    }
			}		
    		} 
    else
	return fp;
    
    return NULL;
    }
FILE *fopenN   ( char *fname, char *mode, int max_n_tries, int delay)
{
  FILE *fp;
  int a;
  
  for (a=0; a< max_n_tries; a++)
    {
      if ((fp=fopen (fname, mode))) return fp;
      else sleep (delay);
      HERE ("---- failed opening: %s", fname);
    }
  return NULL;
}
      
FILE * vfclose ( FILE *fp)
       {
       if ( fp==NFP)return NULL;
       if ( fp==stdout)return stdout;
       if ( fp==stderr)return stderr;
       if ( fp==stdin) return stdin;
       if ( fp==NULL)return NULL;
       else fclose (fp);
       return NULL;
       }
	

int echo ( char *string, char *fname)
{
int a;
/*
description:
prints the content of string into file fname

in:
string= string to print
fname =name of the file to create
*/

FILE *fp;

    fp=vfopen ( fname, "w");
    fprintf (fp, "%s", string);
    a=fclose (fp);
    return a;

}

int   file_cat ( char *from, char *to)
{
  FILE *fp;
  //appends the content of file1 to file 2
  if (!(fp=vfopen (to, "a")))return 0;
  if (!display_file_content (fp, from)) return 0;
  vfclose (fp);
  return 1;
}
  
FILE* display_file_content (FILE *output, char *name)
{
  FILE *fp;
  int c;
  if ( !name || !file_exists (NULL,name) || !(fp=vfopen (name, "r")))return NULL;
  while ( (c=fgetc(fp))!=EOF)fprintf (output,"%c", c);
  vfclose (fp);
  return output;
}

char ***file2list ( char *name, char *sep)
{
  /*Rturns an array where
    list[0]: first line
    list[0][0]: number of words
    list[0][1]:first word;
    list[n]=NULL
  */
  char **lines, ***list; 
  int a, n;

  lines=file2lines (name);
  if (!lines) return NULL;
  else
    {
      n=atoi (lines[0]);

      list=vcalloc ( n+1, sizeof (char**));
      for ( a=1; a<n; a++)
	{
	  
	  list[a-1]=string2list2(lines[a], sep);
	}
    }
  
  free_arrayN((void**)lines, 2);
  return list;
}
char **file2lines (char *name)
{
  /*lines[0]->nlines;
    lines[1]->first_line
  */
  char **lines;
  char *string;


  string=file2string (name);
  if ( !string) return NULL;
  else
    {
      lines=string2list2(string, "\n");
      vfree ( string);
      return lines;
    }
}
int string2file (char *file, char *mode, char *string,...)
{
  FILE *fp;
  va_list ap;
  
  if (!file) return 0;
  else if ( !mode) return 0;
  else if ( !(fp=vfopen (file, mode)))return 0;
  va_start (ap, string);
  vfprintf (fp, string, ap);
  vfclose (fp);
  va_end (ap);
  return 1;
}

char *file2string (char *name)
{
  FILE*fp;
  char *string;
  int a, c;
  
  if (!name || !file_exists (NULL,name))return NULL;
  else
    {
      string=vcalloc ( count_n_char_in_file(name)+1, sizeof (char));
      fp=vfopen (name, "r");a=0;
      while ( (c=fgetc(fp))!=EOF)
	{
	  string[a++]=c;
	}
      string[a]='\0';
      vfclose (fp);
      return string;
    }
}

int get_cl_param (int argc, char **argv, FILE **fp,char *para_name, int *set_flag, char *type, int optional, int max_n_val,char *usage, ...)
        {
	/*
	usage:
	argc:       n_ arg
	argv        list *
	para_name   param
	set_flag    set to 1 if param set;
	para_type   F, I, S, R_FN (read_file, name), W_FN (written file, name), R_FP (pointer)
	max_n_val   maximum number of values;
	optional    1 for yes, 0 for no
	usage       usage list with optional value;
	val         pointer to the varaible holding the value(s)
	default1    default value (if value id not there)
	default2    default value if the flag is there but no value set ("")indicates an error
	range_left  min value ( "any" for any);
	range_right  max_value ( "any" for any);
	*/
	int pos=0;
	int a;
	va_list ap;
	
	int   *int_val=NULL;
	float *float_val=NULL;
	char  **string_val=NULL;

	
	char  *range_right;
	char  *range_left;
	
	
	char *default_value1;
	char *default_value2;
	int n_para=0;        
	double max, min;
	
	static char **parameter_list;
	static int    number_of_parameters;

	char **para_name_list;
	int    n_para_name;
	
	char **para_val;
	int    n_para_val;
	
	char **pv_l=NULL;
	int    n_pv_l;
	char **pv_r=NULL;
	int    n_pv_r;
	char   value[STRING];



/*CHECK THAT ALL THE PARAM IN ARG EXIST*/
	if ( para_name==NULL)
	   {
	   for ( a=1; a< argc; a++)
	       {
	       if ( is_parameter ( argv[a]))
		 
		 if (strstr (argv[a], "help"))myexit (EXIT_SUCCESS);
		 else if ( name_is_in_list ( argv[a], parameter_list, number_of_parameters, STRING)==-1)
		      {
			myexit(fprintf_error ( stderr, "\n%s IS NOT A PARAMETER  OF %s [FATAL/%s %s]\n",argv[a], argv[0], argv[0], VERSION));
		      
		      }
		  
		     
	       }
	 
	   free_char (parameter_list,-1);
	   return 0;
	   }

	if ( parameter_list==NULL)parameter_list=declare_char(MAX_N_PARAM,STRING);
	para_name_list=get_list_of_tokens(para_name,NULL, &n_para_name);
	for ( a=0; a< n_para_name; a++)
	    {
	    sprintf ( parameter_list[number_of_parameters++],"%s", para_name_list[a]);
	    }
	free_char(para_name_list,-1);
        


	
	
	set_flag[0]=0;
	va_start (ap, usage);
	
	if (strm3 (type, "S","R_F","W_F"))
		string_val=va_arg(ap, char**);
	else if (strm2 (type, "D","FL"))
	        int_val=va_arg(ap, int*);
	else if (strm (type, "F"))
	        float_val=va_arg(ap, float*);
	else 
	    myexit (EXIT_FAILURE);
	
	
	
	default_value1=va_arg(ap, char*);
	default_value2=va_arg(ap, char*);
	range_left    =va_arg(ap, char*);
	range_right   =va_arg(ap, char*);
       	va_end(ap);


	para_name_list=get_list_of_tokens(para_name, NULL, &n_para_name);
	for ( a=0; a<n_para_name; a++) 
	    {
	    if ( (pos=name_is_in_list(para_name_list[a], argv,argc, STRING))!=-1)break;
	    }
	free_char (para_name_list,-1);
	
	if (  (name_is_in_list("-help" , argv,argc  ,STRING)!=-1) && (argc==2 || (name_is_in_list( para_name , argv,argc  ,STRING)!=-1)))
	  {
	
	       fprintf ( stderr, "PARAMETER   : %s\n",  para_name);
	       fprintf ( stderr, "USAGE       : %s\n",       usage);
	       fprintf ( stderr, "MAX_N_VALUES: %d\n",   max_n_val);
	       fprintf ( stderr, "DEFAULT     : %s OR %s (when flag set)\n", default_value1, default_value2);
	       fprintf ( stderr, "RANGE       : [%s]...[%s]\n", range_left,(strm(range_right,"any"))?"":range_right);
	       fprintf ( stderr, "TYPE        : %s\n\n", type);
	       return 0;
	  }
	else if ( name_is_in_list ("-help" , argv,argc  ,STRING)!=-1)
	  {
	    return 0;
	  }
	else if (para_name[0]!='-')
	   {
	   myexit(fprintf_error ( stderr, "\nWRONG PARAMETER DEFINITION %s Must Start with a dash", para_name));
	   
	   }
     	else if (pos==-1)
	   {
	   if ( optional==OPTIONAL)
	      {
	      set_flag[0]=0;	      
	      para_val=get_list_of_tokens(default_value1, NULL, &n_para_val);
	      
	      for (n_para=0; n_para<n_para_val && !strm (default_value1, "NULL"); n_para++) 
	          {
		  if ( strm (para_val[n_para], ""))
		      {
		      set_flag[0]=0;
		      break;
		      }
		  else if ( strm (type, "FL"))
	              {
		      set_flag[0]=atoi(para_val[n_para]);
		      break;
		      }
		  else if (strm3 (type, "S", "R_F","W_F"))
		      {
		     
		      sprintf ( string_val[n_para], "%s",para_val[n_para]);
		      }		  
		  else if ( strm (type, "D"))
		      int_val[n_para]=atoi(para_val[n_para]);
		  else if ( strm (type, "F"))
		      float_val[n_para]=atof(para_val[n_para]);
		  }
	      free_char (para_val, -1);
               
	      if (n_para==0 && strm3(type, "S","W_F","R_F") && strm (default_value1, "NULL"))
		  {
		  vfree (string_val[0]);
		  string_val[0]=NULL;
		  
		  }
	      else if (n_para==0 && strm (type, "D") && strm (default_value1, "NULL"))int_val[0]=0;		   
	      else if (n_para==0 && strm (type, "F") && strm (default_value1, "NULL"))float_val[0]=0;
	 
	      }
	   else
	      {
	      myexit(fprintf_error ( stderr, "\nParameter %s is not optional",para_name));
	      
	      }	   
	   }
	else if (pos!=-1)
	  {
	  set_flag[0]=1;
	  for (a=pos+1; a< argc; a++)
	      {
	      if ( is_parameter(argv[a]))break;
	      else
	          {
		  if ( n_para>=max_n_val)
		     {
		     n_para=max_n_val-1;
		     
		     }
		  if ( !(strm ( argv[a], "NULL")))		      	  
		    {
		      if ( strm3(type, "S", "R_F", "W_F"))
			  {
			  sprintf ( string_val[n_para],"%s", argv[a]);
			  }
		       else if (strm (type, "D"))
		          {
			  int_val[n_para]=atoi(argv[a]);
			  }
		       else if (strm ( type,"F"))
		          {
			  float_val[n_para]=atof(argv[a]);
			  }
		    }
		  n_para++;
		  }
	      }

	  if ( n_para==0 && !strm2(default_value2,"","NULL") && !strm(type, "FL"))
	      {
	      para_val=get_list_of_tokens(default_value2, NULL, &n_para_val);
	      for ( n_para=0; n_para<n_para_val; n_para++)
	          {
		  if ( strm3(type, "S", "R_F", "W_F"))sprintf ( string_val[n_para],"%s", para_val[n_para]);		  
		  else if (strm (type, "D"))int_val  [n_para]=atoi(para_val[n_para]);
		  else if (strm ( type,"F"))float_val[n_para]=atof(para_val[n_para]);		
		  }
	      free_char (para_val,-1);
	      }
	  else if (n_para==0 && strm (type, "FL"));
	  else if (n_para==0 && strm3(type, "S","W_F","R_F") && strm (default_value2, "NULL")){vfree (string_val[0]);string_val[0]=NULL;}
	  else if (n_para==0 && strm (type, "D") && strm (default_value2, "NULL"))int_val[0]=0;		   
	  else if (n_para==0 && strm (type, "F") && strm (default_value2, "NULL"))float_val[0]=0;
	  else if (n_para==0 && strm (default_value2, ""))
	         {
		 myexit(fprintf_error ( stderr, "\nParam %s needs a value [FATAL/%s]", para_name, argv[0]));
		 
		 }
	  else;
	  }
	  
/*Check That The Parameters are in the Good Range*/
	
	pv_l=get_list_of_tokens( range_left , NULL, &n_pv_l);
	pv_r=get_list_of_tokens( range_right, NULL, &n_pv_r);

	for ( a=0; a< n_para; a++)
	    {	   
	    if ( strm (type, "R_F") && !check_file_exists(string_val[a]) && !check_file_exists(string_val[a]+1))
			{			
			myexit(fprintf_error ( stderr, "PARAM %s: File %s does not exist [FATAL/%s]\n",para_name,string_val[a], argv[0]));
			
		        }	    
	    else if ( strm (pv_l[0], "any"));
	    else if ( strm (type, "D"))
	         {
		 if ( n_pv_l==1)
		    {
		    min=(double)atoi(pv_l[0]);
		    max=(double)atoi(pv_r[0]);
		    if ( int_val[a]<min || int_val[a]>max)
		       {
		       myexit(fprintf_error ( stderr, "\n%s out of range [%d %d] [FATAL/%s]\n", para_name, (int)min, (int)max,argv[0]));
		       
		       }
		    }
		 else
		    {
		    sprintf ( value, "%d", int_val[a]);
		    if ( name_is_in_list(value, pv_l, n_pv_l, STRING)==-1)
			fprintf ( stderr, "\n%s out of range [%s: ", para_name, value);
		    print_array_char (stderr, pv_l, n_pv_l, " ");
		    fprintf ( stderr, "\n");
		    myexit(EXIT_FAILURE);
		    }
		 }
	    else if ( strm (type, "F"))
	         {
		  if ( n_pv_l==1)
		    {
		    min=(double)atof(range_left);
		    max=(double)atof(range_right);
		    if ( float_val[a]<min || float_val[a]>max)
		       {
		       myexit(fprintf_error ( stderr, "\n%s out of range [%f %f] [FATAL/%s]\n", para_name, (float)min, (float)max,argv[0]));
		       
		       }
		     }
		  else
		     {
		     sprintf ( value, "%f", float_val[a]);
		     if ( name_is_in_list(value, pv_l, n_pv_l, STRING)==-1)
			fprintf ( stderr, "\n%s out of range [%s: ", para_name, value);
		     print_array_char (stderr, pv_l, n_pv_l, " ");
		     fprintf ( stderr, "\n");
		     
		     }
		 }
	    }
         

	 if ( fp[0]!=NULL)
	      {
	      fprintf (fp[0], "%-15s\t%s\t[%d] ", para_name, type, set_flag[0]);
	      for (a=0; a<n_para; a++) 
		  {
		  if ( strm3 ( type, "S", "R_F", "W_F"))fprintf ( fp[0], "\t%s", string_val[a]);
		  else if ( strm  ( type, "D"))fprintf ( fp[0], "\t%d ", int_val[a]);
		  else if ( strm  ( type, "F"))fprintf ( fp[0], "\t%f ", float_val[a]);
	         }	    
	      if ( strm (type, "FL"))fprintf ( fp[0], "\t%d", int_val[0]);
	      fprintf ( fp[0], "\n");     
	      }

	free_char ( pv_l, -1);
	free_char ( pv_r, -1);
	return n_para;
	}

	
	
char ** get_parameter ( char *para_name, int *np, char *fname)
{
    /*
    In:
    para_name: the name of the parameter to look for
    fname: the name of the file containing the parameters
    np[0]: set to 0

    Out:
    char ** containing the np[0] values taken by para_name in fname.
    
    Special:
    if fname=NULL, para_name is searched using the last value taken by fp.
    
    Note: by default, the function keeps a file handle open until the first unsuccessful call.
    */

    static FILE *fp;
    static char *line;
    char ** return_value;
    
    if ( strm (para_name, "CLOSE THE FILE"))
      {
	vfclose ( fp);
	return NULL;
      }
    
    if ( line==NULL)line=vcalloc ( VERY_LONG_STRING+1, sizeof (char));
    if ( fname!=NULL && fp!=NULL)vfclose (fp);
    
    np[0]=0;

    if ((fp=find_token_in_file ( fname,(fname==NULL)?fp:NULL, para_name))==NULL) 
	{
	     return NULL;
	}
    else
        {
	fgets ( line, VERY_LONG_STRING,fp);
        return_value=get_list_of_tokens ( line, NULL, np);	
	return return_value;
	}
}

FILE * set_fp_id ( FILE *fp, char *id)
	{
/*Sets fp just after id, id needs to be at the begining of the line*/
	char string[10000];
	int cont=1;
	int c;
	
	while ( cont==1)	
		{
		c=fgetc(fp);
		if ( c!=EOF)
			{
			
			ungetc(c, fp);
			fscanf ( fp, "%s", string);
			
			if ( strcmp ( string, id)==0)
				return fp;
			else while ( c!='\n' && c!=EOF)
				c=fgetc(fp);			
			}
		else if ( c==EOF)
			{
			fclose ( fp);
			return NULL;
			}
		}	
	return fp;
	}
FILE * set_fp_after_char ( FILE *fp, char x)
	{
/*sets fp just after the first occurence of x*/
	


	int cont=1;
	int c;
	
	while ( cont==1)	
		{
		c=fgetc(fp);
		if ( c!=EOF)
			{
			if ( c==x)
				return fp;
			
			}
		else if ( c==EOF)
			{
			fclose ( fp);
			return NULL;
			}
		}	
	return fp;
	}

FILE * find_token_in_file_nlines ( char *fname, FILE * fp, char *token, int n_line)

        {
	  /*This function finds the string TOKEN (as a single word) in the n_line first lines of fname.
	    It returns NULL if not found or the position of the fp
	  */
	  
	  char *tmp_name=NULL;
	  FILE *fp1;
	  FILE *fp2;
	  char buffer[10001];
	  int a;
	  if ( !fp && !file_exists(NULL,fname))return NULL;
	  if ( fp==NULL)
	    {
	      tmp_name=vtmpnam ( NULL);
	
	      fp1=vfopen (fname, "r");
	      fp2=vfopen (tmp_name, "w");
	      
	      for ( a=0; a< n_line && fgets(buffer, 10000, fp1)!=NULL; a++)
		{
		  fprintf ( fp2, "%s", buffer);
		}
	      vfclose (fp1);
	      vfclose (fp2);
	    }
	  return find_token_in_file ( tmp_name,fp,token);
	}
	  
int token_is_in_file ( char *fname, char *token)
{
  /*TH:an argument against torture: innocents get tortured longer, likewise for token testing*/
  FILE *fp;
  static char *buf;
  char *b, *p;
  int begining;
  

  
  if (token[0]=='\n')
    {

      begining=1;
      token++;
    }
  else
    {
      begining=0;
    }
  
  if ( !fname || !file_exists(NULL,fname))return 0;
  else
    {
      fp=vfopen (fname, "r");
      while ((b=vfgets (buf,fp))!=NULL)
	{
	  buf=b;
	  p=strstr (buf, token);
	  if (!p);
	  else if ( begining==1 && p==buf){vfclose (fp); return 1;}
	  else if ( begining==0 && p){vfclose (fp); return 1;}
	}
      vfclose (fp);
      return 0;
    }
  return 0;
}

char *vfgets ( char *buf, FILE *fp)
{
  int c;
  int buf_len, l;
  char *bufin;
  static int debug;


  if ( !debug)
    debug=(getenv ("DEBUG_TCOFFEE"))?1:-1;
  

  bufin=buf;

  if (!buf)
    {
      buf=vcalloc ( 1000, sizeof (char));
      buf_len=1000;
    }
  else
    {
      buf_len=read_array_size_new (buf);
    }
  
  l=0;
  
  if ( (c=fgetc (fp))==EOF)return NULL;
  else ungetc (c, fp);
  
  
  while ( (c=fgetc (fp))!='\n' && c!=EOF)
    {
      if (l>=buf_len)
	{buf_len+=100;buf=vrealloc (buf, buf_len*sizeof (char));}
      buf[l++]=c;
    }
  /*Add the cariage return*/
  if ( c=='\n')
    {
      if (l>=buf_len){buf_len+=100,buf=vrealloc (buf, buf_len*sizeof (char));}
      buf[l++]='\n';
    }
  /*add the terminator*/  
  if (l>=buf_len){buf_len+=100,buf=vrealloc (buf, buf_len*sizeof (char));}
  buf[l]='\0';

  if ( bufin!=buf && bufin!=NULL && debug==1)
    fprintf ( stderr, "\nPointer change in vfgets...");
  
  return buf;
}
  

FILE * find_token_in_file ( char *fname, FILE * fp, char *token)
	{
	int c;
	static char *name;
	int token_len;

	int only_start;
	
	/*Note: Token: any string
	        If Token[0]=='\n' Then Token only from the beginning of the line
	*/

	if (!fp && !file_exists("CACHE",fname))return NULL;

	if ( token[0]=='\n'){token++;only_start=1;}
	else only_start=0;

	token_len=strlen (token);

	
	
	
	
	if (!fp)
	  {
	    if (name)vfree (name);
	    name = vcalloc (((fname)?measure_longest_line_in_file (fname):10000)+1, sizeof (char));
	    fp=vfopen ( fname, "r");
	  }
	    
	while ( (fscanf ( fp, "%s", name))!=EOF)
		{
		 
		if ( name[0]=='*')while ( ((c=fgetc (fp))!='\n')&& (c!=EOF));
		else if (strncmp ( name, token,token_len)==0){return fp;}
		else if (only_start) while ( ((c=fgetc (fp))!='\n')&& (c!=EOF));
		}

	vfclose ( fp);
	return NULL;
	}
int **get_file_block_pattern (char *fname, int *n_blocks, int max_n_line)
        {
	int c;
	FILE *fp;
	char *line;
	int lline;
	int **l;
	int in_block;
	
	int max_block_size;
	int block_size;
	int x;
	int n_line;
	
	lline=measure_longest_line_in_file (fname)+1;			
	line=vcalloc ( sizeof (char),lline+1);
	
	fp=vfopen (fname, "r");
	max_block_size=block_size=0;
	in_block=1;
	n_blocks[0]=0;
	n_line=0;
	while ((c=fgetc(fp))!=EOF && (n_line<max_n_line || !max_n_line))
	    {
		  ungetc (c, fp);
		  fgets ( line, lline,fp);
		  n_line++;

		  if ( is_alnum_line (line) && !in_block){n_blocks[0]++;in_block=1;}
		  if ( is_alnum_line (line))
		     {
		     block_size++;
		     }
		  else
		      {
		      in_block=0;
		      max_block_size=MAX( max_block_size, block_size);
		      block_size=0;
		      }
	    }

	
	max_block_size=MAX( max_block_size, block_size);
	vfclose ( fp);
	
	l=declare_int (n_blocks[0]+1,max_block_size+1);  

	
	fp=vfopen (fname, "r");	
	in_block=1;
	n_blocks[0]=0;
	n_line=0;
	while ((c=fgetc(fp))!=EOF && (n_line<max_n_line || !(max_n_line)))
	      {
		  ungetc (c, fp);
		  fgets ( line, lline,fp);
		  n_line++;

		  if ( is_alnum_line (line) && !in_block){n_blocks[0]++;in_block=1;}
		  if ( is_alnum_line (line))
		      {
		      l[n_blocks[0]][0]++;
		      free_char (get_list_of_tokens (line, " \t\n*:,", &x), -1); 
		      
		      if ( l[n_blocks[0]][0]> max_block_size)myexit(fprintf_error ( stderr, "\nERROR %d", l[n_blocks[0]][0]));

		      l[n_blocks[0]] [l[n_blocks[0]][0]]=x;
		      }
		  else
		      {
			  in_block=0;
		      }
	      }
	n_blocks[0]++;
	vfree(line);
	vfclose (fp);
	return l;
	}		  

char * strip_file_from_comments (char *com, char *in_file)
{
  /*Removes in file in_file every portion of line to the right of one of the symbols included in com
    Writes the striped file into a vtmpnam file
   */
  FILE *fp1;
  FILE *fp2;
  char *out_file;
  int c;

  out_file=vtmpnam(NULL);


  fp1=vfopen (in_file , "r");
  fp2=vfopen (out_file, "w");
  while ( (c=fgetc(fp1))!=EOF)
	  {
	    if (strchr(com, c))
	      {	
		while ( (c=fgetc(fp1))!='\n' && c!=EOF);
	      }
	    else
	      {
		fprintf (fp2, "%c", c);
		while ( (c=fgetc(fp1))!='\n' && c!=EOF)fprintf (fp2, "%c", c);
		if ( c!=EOF)fprintf (fp2, "%c", c);
	      }
	  }
  vfclose (fp1);
  vfclose (fp2);
 
  return out_file;
}
FILE * skip_commentary_line_in_file ( char com, FILE *fp)
{
  int c=0;

  if ( fp==NULL)return NULL;
  while ((c=fgetc(fp))==com)
    {
      while ((c=fgetc(fp))!='\n' && c!=EOF);
    }
  if ( c!=EOF && c!='\n')ungetc(c, fp);
  return fp;
}
     
int check_for_update ( char *web_address)
{
  char command[1000];
  char *file;
  float new_version, old_version;
  FILE *fp;
  
  check_internet_connection (IS_NOT_FATAL);
  file=vtmpnam(NULL);
   
  sprintf ( command, "%s/%s.version",DISTRIBUTION_ADDRESS, PROGRAM);
  url2file ( command, file);
    
  fp=vfopen ( file, "r");
  fscanf ( fp, "Version_%f", &new_version);
  vfclose ( fp);
  sscanf ( VERSION, "Version_%f", &old_version);

  if ( old_version<new_version)
    {
      fprintf ( stdout, "\nUpdate Status: outdated");
      fprintf ( stdout, "\nYour version of %s is not up to date", PROGRAM);
      fprintf ( stdout, "\nDownload the latest version %.2f with the following command:", new_version);
      fprintf ( stdout, "\n\twget %s/%s_distribution_Version_%.2f.tar.gz\n", DISTRIBUTION_ADDRESS, PROGRAM, new_version);
      return EXIT_FAILURE;
    }
  else if ( old_version> new_version)
    {
      fprintf ( stdout, "\nUpdate Status: beta-release");
      fprintf ( stdout, "\nYour are using a beta-release of %s(%s)\n", PROGRAM, VERSION);
    }
  else
    {
      fprintf (stdout, "\nUpdate Status: uptodate");
      fprintf (stdout, "\nProgram %s(%s) is up to date\n", PROGRAM, VERSION);
    }
  return EXIT_SUCCESS;
}
	
      
  
  
  
int check_environement_variable_is_set ( char *variable, char *description, int fatal)
{
  if ( getenv (variable)==NULL)
    {
      myexit(fprintf_error ( stderr, "\nERROR: You must set %s\n%s %s", variable, description, description));
      if ( fatal==IS_FATAL)
	{
	myexit(fprintf_error ( stderr, "\n[%s:FATAL]\n", PROGRAM));
	
	}
      else
	add_warning ( stderr, "\n[%s:WARNING]\n", PROGRAM);
    }
  return 1;
}

int url2file (char *address, char *out)
{
   
  if      (check_program_is_installed ("wget",NULL, NULL,WGET_ADDRESS, IS_NOT_FATAL))return printf_system( "wget %s -O%s >/dev/null 2>/dev/null", address, out);
  else if (check_program_is_installed ("curl",NULL, NULL,CURL_ADDRESS, IS_NOT_FATAL))return printf_system("curl %s -o%s >/dev/null 2>/dev/null", address, out);
  else 
    {
      printf_exit (EXIT_FAILURE, stderr, "ERROR: Impossible to fectch external file: Neither wget nor curl is installed on your system [FATAL:%s]\n", PROGRAM);
      return EXIT_FAILURE;
    }
}

int wget (char *address, char *out)
{
  return printf_system ( "curl %s -O%s >/dev/null 2>/dev/null", address, out);
  }

int curl (char *address, char *out)
{
  return printf_system ( "curl %s -o%s >/dev/null 2>/dev/null", address, out);
}
	

int simple_check_internet_connection (char *ref_site)
{
  char *test,command[1000];
  int n, internet=0;
  
  test=vtmpnam (NULL);
  if (url2file((ref_site)?ref_site:TEST_WWWSITE_4_TCOFFEE,test)!=EXIT_SUCCESS)internet=0;
  else if ((n=count_n_char_in_file(test))<10)internet=0;
  else internet =1;
  
  return internet;
}
int check_internet_connection  (int mode)
{
  int internet;
  internet=simple_check_internet_connection (NULL);
  if (internet)return 1;
  else if ( mode==IS_NOT_FATAL)return internet;
  else proxy_msg(stderr);
  myexit (EXIT_FAILURE);
}
char *pg2path (char *pg)
{
  char *path;
  char *p;
  char *tmp;
  FILE *fp;
    
  if ( !pg) return NULL;
  tmp=vtmpnam(NULL);
  
  printf_system_direct("which %s>%s 2>/dev/null", pg, tmp);
  path=file2string (tmp);
  chomp (path);
  if (!file_exists (NULL,path) && !strstr (pg, ".exe"))
    {
      char pg2[1000];
      sprintf ( pg2, "%s.exe", pg);
      path=pg2path (pg2);
    }
  
  return path;
}
  

int check_program_is_installed ( char *program_name, char *path_variable, char *path_variable_name, char *where2getit, int fatal)
  {
   char command[LONG_STRING];
   static char *path;
   

   if ( strm (where2getit, "built_in"))return 1;
   
   if (path)vfree (path);
   
   if ( check_file_exists (path_variable))
     {
       return 1;
     }
   else 
     {
       
       path=pg2path (program_name);
       if (path && path[0])return 1;
       else
	 {
	   int install=EXIT_FAILURE;
	       
	   if (fatal==INSTALL || fatal==INSTALL_OR_DIE)
	     {
	       HERE ("************** %s is missing from your system. T-Coffee will make an attempt to install it.\n", program_name);
	       install=printf_system ("install.pl %s -plugins=%s -clean", program_name, get_plugins_4_tcoffee());
	     }
	   if ( install==EXIT_SUCCESS)return 1;
	   else if ( fatal==INSTALL)return 0;
	   else if ( fatal==NO_REPORT)return 0;
	   
	   if (fatal==IS_FATAL || fatal==INSTALL_OR_DIE)check_configuration4program();
	   
	   fprintf ( stderr, "\n#*****************************************************************");
	   if (fatal) fprintf_error ( stderr, "\n#ERROR [FATAL:%s]", PROGRAM);
	   else fprintf ( stderr, "\n#WARNING [%s]", PROGRAM);
	   fprintf ( stderr, "\n# The Program %s Needed by %s Could not be found", program_name, PROGRAM);
	   fprintf ( stderr, "\n# If %s is installed on your system:", program_name);
	   fprintf ( stderr, "\n#\t     -Make sure %s is in your $path:",program_name);
	   
	   fprintf ( stderr, "\n# If %s is NOT installed obtain a copy from:", program_name);
	   fprintf ( stderr, "\n#\t%s\n#\n#",where2getit); 
	   fprintf ( stderr, "\n# and install it manualy");
	   fprintf ( stderr, "\n******************************************************************\n");
	 }
     }
   if ( fatal==IS_FATAL || fatal==INSTALL_OR_DIE) myexit (EXIT_FAILURE);
   return 0;
  }

FILE * display_output_filename ( FILE *io, char *type, char *format, char *name, int check_output)
{
  static char ***buf;
  static int nbuf;
  
  if ( strm ( name, "stdout") || strm (name, "stderr"))return io;
  
  if ( check_output==STORE)
    {
      int a;
      if ( buf==NULL)buf=vcalloc ( 1000, sizeof (char**));
      
      for (a=0; a<nbuf; a++)
	if ( strm (name, buf[a][2]))return io;
      
      buf[nbuf]=declare_char (3, 1000);
      sprintf ( buf[nbuf][0], "%s", type);
      sprintf ( buf[nbuf][1], "%s", format);
      sprintf ( buf[nbuf][2], "%s", name);
      nbuf++;
      return io;
    }
  else if ( check_output==FLUSH)
    {
      int a;
      for ( a=0; a< nbuf; a++)
	{
	  io=display_output_filename ( io, buf[a][0], buf[a][1], buf[a][2], CHECK);
	  free_char (buf[a], -1);
	}
      nbuf=0;
    }
  else if ( check_output==CHECK)
    {
      if (check_file_exists(name)==NULL)
	{
	  if ( !strm (name, "no") && !strm (name, "NO") && !strm (name, "STDOUT") && !strm(name, "stdout"))
	       add_warning( io, "\n\t#### File Type= %10s Format= %10s Name= %s | NOT PRODUCED [WARNING:%s:%s]\n",type, format, name, PROGRAM, VERSION );
	  return io;
	}
      fprintf ( io, "\n\t#### File Type= %10s Format= %10s Name= %s",type, format, name );
    }
  return io;
}

FILE * display_input_filename ( FILE *io, char *type, char *format, char *name, int check_output)
{
  if ( check_output==CHECK && check_file_exists(name)==NULL)
    {
      fprintf ( io, "\n\tIIII INPUT File Type= %10s Format= %10s Name= %s | NOT PRODUCED [WARNING:%s:%s]\n",type, format, name, PROGRAM, VERSION );
      return io;
    }
  fprintf ( io, "\n\t#### File Type= %10s Format= %10s Name= %s",type, format, name );
  return io;
}

int file_is_empty (char *fname)
{
  struct stat s;
  if (!fname) return 1;

  stat (fname, &s);
  if (s.st_size)return 0;
  else return 1;
  }
int file_exists (char *path, char *fname)
{
  struct stat s;
  char file[1000];
  
  if (!fname)return 0;
  else if (path && strm (path, "CACHE"))
    {
      if (file_exists (NULL, fname))return 1;
      else return file_exists (get_cache_dir(), fname);
    }
  else if (path) sprintf ( file, "%s/%s", path, fname);
  else if (!path)sprintf (file, "%s", fname);
    
  if (stat(file,& s)!=-1)
    return S_ISREG(s.st_mode);
  else return 0;
}

int isdir  (char *file)
{
  struct stat s;
  if (stat (file,&s)!=-1)
    return S_ISDIR(s.st_mode);
  else return 0;
}
int rrmdir (char *s)
{
  if (isdir(s))return printf_system_direct ("rm -r %s", s);
  return EXIT_FAILURE;
}
    
int isexec (char *file)
{
  char *state;
  
  state=ls_l(NULL,file);
  if (state[0]==0) return 0;
  
  if ( state[0]=='d') return 0;
  if ( state[3]=='x') return 1;
  if ( state[6]=='x') return 1;
  if ( state[9]=='x') return 1;
  return 0;
}

char *ls_l ( char *path,char *file)
{
  char *tmpfile;
  static char *state;
  char command[10000];
  FILE *fp;
  int a;
  
  tmpfile=vtmpnam (NULL);
  if (!state)
    {
  
      state=vcalloc (100, sizeof (char));
    }
  for (a=0;a<100; a++)state[a]=0;
  if (!file)return state;
  
  
  printf_system_direct ( command, "ls -l %s%s%s >%s 2>/dev/null", (path!=NULL)?path:"", (path!=NULL)?"/":"",file, tmpfile);
  
  fp=vfopen (tmpfile, "r");
  if (!fscanf ( fp, "%s", state))
    {
      vfclose(fp); return 0;
    }
  vfclose (fp);
  return state;
}

int my_mkdir ( char *dir_in)
{
  
  int dir_sep='/';
  
  int a, buf;
  char *dir;

  dir=vcalloc ( strlen (dir_in)+strlen (get_home_4_tcoffee())+100, sizeof (char));
  sprintf ( dir, "%s", dir_in);
  tild_substitute ( dir, "~",get_home_4_tcoffee());
  
      
  
  a=0;

  while (dir[a]!='\0')
    {
      
      if ( dir[a]==dir_sep || dir[a+1]=='\0')
	{
	  buf= dir[a+1];
	  dir[a+1]='\0';

	  if (access(dir, F_OK)==-1)
	    {
	    
	      char command [1000];
	      printf_system_direct("mkdir %s", dir);
	      if ( access (dir, F_OK)==-1)
		{
		  myexit(fprintf_error ( stderr, "\nERROR: Could Not Create Directory %s [FATAL:%s]", dir, PROGRAM));		}
	    }
	  dir[a+1]=buf;
	} 
      a++;
    }

  vfree (dir);
  return 1;
}

int filename_is_special (char *fname)
{
  if ( strm5 (fname, "default", "stdin", "stdout","stderr", "/dev/null"))return 1;
  if ( strm3 (fname, "STDIN", "STDOUT", "STDERR"))return 1;
  return 0;
}

char* check_file_exists ( char *fname_in)
	{
	FILE *fp;
	static char *fname1;
	static char *fname2;
	
	
	if (!fname_in)return NULL;
	if (!fname_in[0])return NULL;
	if (fname_in[0]=='-')return NULL;
	
	if (!fname1){fname1=vcalloc (1000, sizeof (char));}
	if (!fname2){fname2=vcalloc (1000, sizeof (char));}
	
	sprintf ( fname1, "%s", fname_in);tild_substitute (fname1, "~", get_home_4_tcoffee());
	sprintf ( fname2, "%s%s", get_cache_dir(),fname1);

	if ( filename_is_special (fname1))return fname1;
	if ( strm5 (fname1, "no", "NO", "No", "NO_FILE","no_file"))return NULL/*fname1*/;
	if (!file_exists( NULL,fname1)) 
	  {
	    if (!file_exists (NULL,fname2))return NULL;
	    else return fname2;
	  }
	else return fname1;
	return NULL;
	}	


void create_file ( char *name)
	{
	FILE *fp;
	
	fp=fopen (name, "w");
	fclose (fp);
	}
void delete_file ( char *fname)
	{

	FILE * fp;
	
	fp=fopen ( fname, "w");
	fprintf ( fp, "x");
    	fclose ( fp);
	
	printf_system_direct ("rm %s", fname);
	}	

int util_rename ( char *from, char *to)
        {
	FILE *fp_from;
	FILE *fp_to;
	int c;

	
	if ( !check_file_exists (from))return 0;
        else if ( check_file_exists (to) && !vremove (to) && !rename ( from, to)==0 );
        else
                {
		 
	        fp_from=vfopen ( from, "r");
		fp_to=vfopen ( to, "w");
		
		while ( (c=fgetc (fp_from))!=EOF)fprintf ( fp_to, "%c", c);

		fclose (fp_from);
		fclose ( fp_to);

		vremove ( from);
		return 1;
		}
	return 0;
	}


int util_copy (  char *from, char *to)
        {
	FILE *fp_from;
	FILE *fp_to;
	int c;

	
	if (!check_file_exists (from))return 0;
        else
                {
		 
	        fp_from=vfopen ( from, "r");
		fp_to=vfopen ( to, "w");
		
		while ( (c=fgetc (fp_from))!=EOF)fprintf ( fp_to, "%c", c);

		fclose (fp_from);
		fclose ( fp_to);
		return 1;
		}
	return 0;
	} 
FILE * output_completion4halfmat ( FILE *fp,int n, int tot, int n_reports, char *s)

{
  int max, left, achieved;
  int up;
  
  if (n>=0)up=1;
  else up=-1;
  

  max=((tot*tot)-tot)/2;
  left=((tot-n)*(tot-n)-(tot-n))/2;
  
  achieved=max-left;
  if (up==1);
  else
    {
      int b;
      b=achieved;
      achieved=left;
      left=b;
    }
  return output_completion (fp,achieved, max, n_reports, s);
}
  
  
FILE * output_completion ( FILE *fp,int n, int tot, int n_reports, char *string)
        {

	  static int ref_val;
	  static int flag;
	  static int ref_time;
	  int t, elapsed;
	  n++;
	  
	  if ( n==1)
	    {
	      ref_val=flag=0;
	      ref_time=get_time()/1000;
	    }
	  t=get_time()/1000;
	  elapsed=t-ref_time;
	    
	  if ( !ref_val && !flag)
	    {
	      fprintf (fp, "\n\t\t[%s][TOT=%5d][%3d %%][ELAPSED TIME: %4d sec.]",(string)?string:"",tot,(tot==1)?100:0, elapsed);
	      flag=1;
	    }
	  else if ( n==tot)fprintf (fp, "\r\t\t[%s][TOT=%5d][%3d %%][ELAPSED TIME: %4d sec.]",(string)?string:"", tot,100, elapsed);
	  else if ( ((n*100)/tot)>ref_val)
	    {
	      ref_val=((n*100)/tot);
	      t=(ref_val==0)?0:elapsed/ref_val;
	      t=t*(100-ref_val);
	      t=0;
	      fprintf (fp, "\r\t\t[%s][TOT=%5d][%3d %%][ELAPSED TIME: %4d sec.]", (string)?string:"",tot,ref_val, elapsed);
	      flag=0;
	    }	   
	  return fp;
	}
void * null_function (int a,...)
{
  myexit(fprintf_error ( stderr, "\n[ERROR] Attempt to use the Null Function [FATAL:%s]", PROGRAM));
  
  return NULL;
}

int  btoi ( int nc,...)
{
  va_list ap;
  int a, b;
  va_start (ap, nc);
  for ( a=0, b=0; a< nc; a++)
    {
      b+=pow(2,a)*va_arg (ap,int);
    }
  va_end(ap);
  return b;
}

/*********************************************************************/
/*                                                                   */
/*                         Geometric FUNCTIONS                    */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

float get_geometric_distance ( float ** matrix, int ncoor, int d1, int d2, char *mode)
{
  float d;
  float t=0;
  int a;

  if ( strm (mode, "euclidian"))
    {
      for ( a=0; a< ncoor; a++)
	{
	  d=(matrix[d1][a]-matrix[d2][a]);
	  t+=d*d;
	}
      return (float)sqrt((double)t);
    }
  return 0;
}
       


/*********************************************************************/
/*                                                                   */
/*                         MATHEMATICAL FUNCTIONS                    */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
static double EXP_UNDERFLOW_THRESHOLD = -4.60f;
static double LOG_UNDERFLOW_THRESHOLD = 7.50f;
static double LOG_ZERO = -FLT_MAX;
static double LOG_ONE = 0.0f;
double log_addN (int N, double*L)
     
{
  double v;
  int a;
  if (N==0)return 0;
  if ( N==1)return L[0];
  
  v=L[0];
  for ( a=1; a<N; a++)
    {
      v=log_add2(v, L[a]);
    }
  return v;
  }
double log_add6 (double x, double y, double z, double w, double v, double e)
{
  return log_add2(log_add3(x, y, z),log_add3(z,w,e));
}
double log_add5 (double x, double y, double z, double w, double v)
{
  return log_add3(log_add2(x, y),log_add2(z,w),v );
}
double log_add4 (double x, double y, double z, double w)
{
  return log_add2(log_add2(x, y),log_add2(z,w));
}
double log_add3 (double x, double y, double z)
{
  return log_add2(log_add2(x, y),z);
}
double log_add2 ( double x, double y)
{
  if (x < y)
    x = (x == LOG_ZERO || ((y - x) >= LOG_UNDERFLOW_THRESHOLD)) ? y : log (exp (x-y) + 1) + x;
  else
    x = (y == LOG_ZERO || ((x - y) >= LOG_UNDERFLOW_THRESHOLD)) ? x : log (exp (x-y) + 1) + y;
  return x;
}


  
  
float M_chooses_Nlog ( int m, int N)
{
  /*Choose M elemets in N*/
  float  z1, z2,z=0;
  if ( m==N) return 0;
  else if ( m>N)
    {
      myexit(fprintf_error ( stderr, "\nERROR: M chosses N out of bounds ( M>N) [FATAL:%s]", PROGRAM));
      myexit (EXIT_FAILURE);
    }
  else
    {
      z1=factorial_log (m+1, N);
      z2=factorial_log (1, N-m);
      z=z1-z2;
      return z;
    }
  
  return -1;
}  

float factorial_log ( int start, int end)
{
  if ( end==0)return 0;
  else if ( end==start) return (float)my_int_log((double)start);
  else if ( start>end)
     {
       fprintf_error ( stderr, "\nERROR: factorial log out of bounds (%d %d) [FATAL:%s]",start, end, PROGRAM);
       myexit (EXIT_FAILURE);
    }
  else
    {
      int a=0;
      float x=0;
      for ( x=0,a=start; a<=end; a++)
	{
	  x+=(float)my_int_log(a);
	}
      return x;
    }
  return 0;
}

float my_int_log(int a)
{
  
  if ( a>=100000)return log(a);
  else
    {
      static float *lu;
      if (!lu) lu=vcalloc ( 100000, sizeof (float));
      if ( !lu[a]){lu[a]=log(a);}
      return lu[a];
    }
  return 0;
}

double factorial (int start, int end);
double M_chooses_N ( int m, int N)
{
  /*Choose M elemets in N*/
  if ( m==N) return 1;
  else if ( m>N)
    {
      fprintf_error ( stderr, "\nERROR: M chosses N out of bounds ( M>N) [FATAL:%s]", PROGRAM);
      myexit (EXIT_FAILURE);
    }
  else if ( N<50)
    {
      return factorial (m+1, N)/factorial (1, N-m);
    }
  else
    {
      fprintf_error ( stderr, "\nERROR: M chosses N out of bounds ( N>50). Use log space [FATAL:%s]", PROGRAM);
      myexit (EXIT_FAILURE);
    }
  return -1;
}
double factorial (int start, int end)
  {
    
    if ( start>end || start<0 || end<0)
      {
	fprintf_error ( stderr, "\nERROR: Negative Factorial [FATAL:%s]", PROGRAM);
	myexit ( EXIT_FAILURE);
      }
    else if (end==0) return 1;
    else if (end==start) return end;
    else
      {
	static double **lu;
	if ( !lu)lu=declare_double (100, 100);
	
	if ( lu[start][end])return lu[start][end];
	else
	  {
	    int a;
	    lu[start][end]=(double)start;
	    for ( a=start+1; a<=end; a++)
	      {
		lu[start][end]*=(double)a;
	      }
	    return  lu[start][end];
	  }
      }
    return -1;
  }
/*********************************************************************/
/*                                                                   */
/*                         Fast Log Additions (adapted from Probcons)*/
/*                                                                   */
/*                                                                   */
/*********************************************************************/
double EXP (double x){
  //return exp(x);
  if (x > -2){
    if (x > -0.5){
      if (x > 0)
	return exp(x);
      return (((0.03254409303190190000*x + 0.16280432765779600000)*x + 0.49929760485974900000)*x + 0.99995149601363700000)*x + 0.99999925508501600000;
    }
    if (x > -1)
      return (((0.01973899026052090000*x + 0.13822379685007000000)*x + 0.48056651562365000000)*x + 0.99326940370383500000)*x + 0.99906756856399500000;
    return (((0.00940528203591384000*x + 0.09414963667859410000)*x + 0.40825793595877300000)*x + 0.93933625499130400000)*x + 0.98369508190545300000;
  }
  if (x > -8){
    if (x > -4)
      return (((0.00217245711583303000*x + 0.03484829428350620000)*x + 0.22118199801337800000)*x + 0.67049462206469500000)*x + 0.83556950223398500000;
    return (((0.00012398771025456900*x + 0.00349155785951272000)*x + 0.03727721426017900000)*x + 0.17974997741536900000)*x + 0.33249299994217400000;
  }
  if (x > -16)
    return (((0.00000051741713416603*x + 0.00002721456879608080)*x + 0.00053418601865636800)*x + 0.00464101989351936000)*x + 0.01507447981459420000;
  return 0;
}

float LOOKUP (float x){

  if (x <= 1.00f) return ((-0.009350833524763f * x + 0.130659527668286f) * x + 0.498799810682272f) * x + 0.693203116424741f;
  if (x <= 2.50f) return ((-0.014532321752540f * x + 0.139942324101744f) * x + 0.495635523139337f) * x + 0.692140569840976f;
  if (x <= 4.50f) return ((-0.004605031767994f * x + 0.063427417320019f) * x + 0.695956496475118f) * x + 0.514272634594009f;
  
  return ((-0.000458661602210f * x + 0.009695946122598f) * x + 0.930734667215156f) * x + 0.168037164329057f;
}
void LOG_PLUS_EQUALS (float *x, float y){
  
  if (x[0] < y)
    x[0] = (x[0] == LOG_ZERO || y - x[0] >= LOG_UNDERFLOW_THRESHOLD) ? y : LOOKUP(y-x[0]) + x[0];
  else
    x[0] = (y == LOG_ZERO || x[0] - y >= LOG_UNDERFLOW_THRESHOLD) ? x[0]  : LOOKUP(x[0]-y) + y;
}

float LOG_ADD (float x, float y){
  if (x < y) return (x == LOG_ZERO || y - x >= LOG_UNDERFLOW_THRESHOLD) ? y : LOOKUP(y-x) + x;
  return (y == LOG_ZERO || x - y >= LOG_UNDERFLOW_THRESHOLD) ? x : LOOKUP(x-y) + y;
}

float LOG_ADD3 (float x1, float x2, float x3){
  return LOG_ADD (x1, LOG_ADD (x2, x3));
}
float LOG_ADD4 (float x1, float x2, float x3, float x4){
  return LOG_ADD (x1, LOG_ADD (x2, LOG_ADD (x3, x4)));
}
float LOG_ADD5 (float x1, float x2, float x3, float x4, float x5){
  return LOG_ADD (x1, LOG_ADD (x2, LOG_ADD (x3, LOG_ADD (x4, x5))));
}
float LOG_ADD6 (float x1, float x2, float x3, float x4, float x5, float x6){
  return LOG_ADD (x1, LOG_ADD (x2, LOG_ADD (x3, LOG_ADD (x4, LOG_ADD (x5, x6)))));
}
float LOG_ADD7 (float x1, float x2, float x3, float x4, float x5, float x6, float x7){
  return LOG_ADD (x1, LOG_ADD (x2, LOG_ADD (x3, LOG_ADD (x4, LOG_ADD (x5, LOG_ADD (x6, x7))))));
}


#define LONG_SIZE 2
#define SHORT_SIZE 1
#define SPACE_PAD 4
#define STD_SIZE 0    
char *strscn(char *s, char *pattern);
long unsigned strtou(char *s, int base, char **scan_end);
long int strtoi(char *s, int base, char **scan_end);
int my_isnumber(char c, int base);
int tonumber(char c);

int my_vsscanf(char *buf, char *fmt, va_list parms)
 {
         int scanned = 0, size = 0, suppress = 0;
         int w = 0, flag = 0, l = 0;
         char c, *c_ptr;
         long int n1, *n1l;
         int *n1b;
         short int *n1s;
         long unsigned n2, *n2l, parsing = 0;
         unsigned *n2b;
         short unsigned *n2s;
         double n3, *n3l;
         float *n3s;
         char *base = buf;
         while (*fmt != 0) {
                 if (*fmt != '%' && !parsing) {
                         /* No token detected */
                         fmt++;
                 } else {
                         /* We need to make a conversion */
                         if (*fmt == '%') {
                                 fmt++;
                                 parsing = 1;
                                 size = STD_SIZE;
                                 suppress = 0;
                                 w = 0;
                                 flag = 0;
                                 l = 0;
                         }
                         /* Parse token */
                         switch (*fmt) {
                         case '1':
                         case '2':
                         case '3':
                         case '4':
                         case '5':
                         case '6':
                         case '7':
                         case '8':
                         case '9':
                         case '0':
                                 if (parsing == 1) {
                                         w = strtou(fmt, 10, &base);
                                         /* We use SPACE_PAD to parse %10s
                                          * commands where the number is the
                                          * maximum number of char to store!
                                          */
                                         flag |= SPACE_PAD;
                                         fmt = base - 1;
                                 }
                                 break;
                         case 'c':
                                 c = *buf++;
                                 c_ptr = va_arg(parms, char *);
                                 *c_ptr = c;
                                 scanned++;
                                 parsing = 0;
                                 break;
                         case 's':
                                 c_ptr = va_arg(parms, char *);
                                 while (*buf != 0 && isspace(*buf))
                                         buf++;
                                 l = 0;
                                 while (*buf != 0 && !isspace(*buf)) {
                                         if (!(flag & SPACE_PAD))
                                                 *c_ptr++ = *buf;
                                         else if (l < w) {
                                                 *c_ptr++ = *buf;
                                                 l++;
                                         }
                                         buf++;
                                 }
                                 *c_ptr = 0;
                                 scanned++;
                                 parsing = 0;
                                 break;
                         case 'i':
                         case 'd':
                                 buf = strscn(buf, "1234567890-+");
                                 n1 = strtoi(buf, 10, &base);
                                 buf = base;
                                 if (!suppress) {
                                         switch (size) {
                                         case STD_SIZE:
                                                 n1b = va_arg(parms, int *);
                                                 *n1b = (int) n1;
                                                 break;
                                         case LONG_SIZE:
                                                 n1l = va_arg(parms,
                                                              long int *);
                                                 *n1l = n1;
                                                 break;
                                         case SHORT_SIZE:
                                                 n1s = va_arg(parms,
                                                              short int *);
                                                 *n1s = (short) (n1);
                                                 break;
                                         }
                                         scanned++;
                                 }
                                 parsing = 0;
                                 break;
                         case 'u':
                                 buf = strscn(buf, "1234567890");
                                 n2 = strtou(buf, 10, &base);
                                 buf = base;
                                 if (!suppress) {
                                         switch (size) {
                                         case STD_SIZE:
                                                 n2b = va_arg(parms,
                                                              unsigned *);
                                                 *n2b = (unsigned) n2;
                                                 break;
                                         case LONG_SIZE:
                                                 n2l = va_arg(parms,
                                                              long unsigned *);
                                                 *n2l = n2;
                                                 break;
                                         case SHORT_SIZE:
                                                 n2s = va_arg(parms, short unsigned
                                                              *);
                                                 *n2s = (short) (n2);
                                                 break;
                                         }
                                         scanned++;
                                 }
                                 parsing = 0;
                                 break;
                         case 'x':
                                 buf = strscn(buf, "1234567890xabcdefABCDEF");
                                 n2 = strtou(buf, 16, &base);
                                 buf = base;
                                 if (!suppress) {
                                         switch (size) {
                                         case STD_SIZE:
                                                 n2b = va_arg(parms,
                                                              unsigned *);
                                                 *n2b = (unsigned) n2;
                                                 break;
                                         case LONG_SIZE:
                                                 n2l = va_arg(parms,
                                                              long unsigned *);
                                                 *n2l = n2;
                                                 break;
                                         case SHORT_SIZE:
                                                 n2s = va_arg(parms, short unsigned
                                                              *);
                                                 *n2s = (short) (n2);
                                                 break;
                                         }
                                         scanned++;
                                 }
                                 parsing = 0;
                                 break;
                         case 'f':
                         case 'g':
                         case 'e':
                                 buf = strscn(buf, "1234567890.e+-");
                                 n3 = strtod(buf, &base);
                                 buf = base;
                                 if (!suppress) {
                                         switch (size) {
                                         case STD_SIZE:
                                                 n3l = va_arg(parms, double *);
                                                 *n3l = n3;
                                                 break;
                                         case LONG_SIZE:
                                                 n3l = va_arg(parms, double *);
                                                 *n3l = n3;
                                                 break;
                                         case SHORT_SIZE:
                                                 n3s = va_arg(parms, float *);
                                                 *n3s = (float) (n3);
                                                 break;
                                         }
                                         scanned++;
                                 }
                                 parsing = 0;
                                 break;
                         case 'l':
                                 size = LONG_SIZE;
                                 break;
                         case 'h':
                         case 'n':
                                 size = SHORT_SIZE;
                                 break;
                         case '*':
                                 suppress = 1;
                                 break;
                         default:
                                 parsing = 0;
                                 break;
                         }
                         fmt++;
                 }
         }
         return (scanned);
 }
char *strscn(char *s, char *pattern)
 {
         char *scan;
         while (*s != 0) {
                 scan = pattern;
                 while (*scan != 0) {
                         if (*s == *scan)
                                 return (s);
                         else
                                 scan++;
                 }
                 s++;
         }
         return (NULL);
 }

long unsigned strtou(char *s, int base, char **scan_end)
 {
         int value, overflow = 0;
         long unsigned result = 0, oldresult;
         /* Skip trailing zeros */
         while (*s == '0')
                 s++;
         if (*s == 'x' && base == 16) {
                 s++;
                 while (*s == '0')
                         s++;
         }
         /* Convert number */
         while (my_isnumber(*s, base)) {
                 value = tonumber(*s++);
                 if (value > base || value < 0)
                         return (0);
                 oldresult = result;
                 result *= base;
                 result += value;
                 /* Detect overflow */
                 if (oldresult > result)
                         overflow = 1;
         }
         if (scan_end != 0L)
                 *scan_end = s;
         if (overflow)
                 result = INT_MAX;
         return (result);
 }
long int strtoi(char *s, int base, char **scan_end)
 {
         int sign, value, overflow = 0;
         long int result = 0, oldresult;
         /* Evaluate sign */
         if (*s == '-') {
                 sign = -1;
                 s++;
         } else if (*s == '+') {
                 sign = 1;
                 s++;
         } else
                 sign = 1;
         /* Skip trailing zeros */
         while (*s == '0')
                 s++;
         /* Convert number */
         while (my_isnumber(*s, base)) {
                 value = tonumber(*s++);
                 if (value > base || value < 0)
                         return (0);
                 oldresult = result;
                 result *= base;
                 result += value;
                 /* Detect overflow */
                 if (oldresult > result)
                         overflow = 1;
         }
         if (scan_end != 0L)
                 *scan_end = s;
         if (overflow)
                 result = INT_MAX;
         result *= sign;
         return (result);
 }

int my_isnumber(char c, int base)
 {
         static char *digits = "0123456789ABCDEF";
         if ((c >= '0' && c <= digits[base - 1]))
                 return (1);
         else
                 return (0);
 }

 int tonumber(char c)
 {
         if (c >= '0' && c <= '9')
                 return (c - '0');
         else if (c >= 'A' && c <= 'F')
                 return (c - 'A' + 10);
         else if (c >= 'a' && c <= 'f')
                 return (c - 'a' + 10);
         else
                 return (c);
 }

///////////////////////////////////////////////////////////////////////////////////////////
// Hash function
////////////////////////////////////////////////////////////////////////////////////////////
unsigned long hash_file(char* file)  //returns the hash value for key 
    {
      // Calculate a hash value by the division method: 
      // Transform key into a natural number k = sum ( key[i]*128^(L-i) ) and calculate i= k % num_slots. 
      // Since calculating k would lead to an overflow, i is calculated iteratively 
      // and at each iteration the part divisible by num_slots is subtracted, i.e. (% num_slots is taken).
      
      unsigned long i=0;     // Start of iteration: k is zero
      unsigned long num_slots=999999999;
      

      FILE *fp;
      unsigned long c;

      
      if (file==NULL || !check_file_exists (file) ) {printf("Warning from util.c:hasch_file: No File [FATAL:%s]\n", PROGRAM); myexit (EXIT_FAILURE);}
      num_slots/=128;
      fp=vfopen (file, "r");
      while ( (c=fgetc (fp))!=EOF)
	{
	  i = ((i<<7) + c) % num_slots; 
	}
      vfclose (fp);
	      
      return i;
    }
int ** r_generate_array_int_list ( int len, int min, int max,int step, int **array, int f, int *n,FILE *fp, int *c_array);
int **generate_array_int_list (int len, int min, int max, int step, int *n, char *file)
   {
     int **array, *c_array;
     FILE *fp=NULL;
     
     if (n==NULL)
       {
	 array=NULL;
	 fp=vfopen (file, "w");
       }
     else
       {
	 int a,s;
	 n[0]=0;
	 for (s=1, a=0; a<len; a++)s*=((max-min)+1)/step;
	 array=declare_int (s, len+1);
       }
     c_array=vcalloc (len, sizeof (int));
     array=r_generate_array_int_list ( len, max, min, step, array, 0, n,fp, c_array);
     vfree (c_array);
     if ( fp) vfclose (fp);
     return array;
   }
int ** r_generate_array_int_list ( int len, int min, int max,int step, int **array, int f, int *n,FILE *fp, int *c_array)
   {
     int a;
     
     if ( f==len)
       {
	 if ( array)
	   {
	     for (a=0; a<len; a++)
	       {
		 array[n[0]][a]=c_array[a];
	       }
	     n[0]++;
	   }
	 else
	   {
	     

	     for (a=0; a<len; a++)fprintf ( fp, "%3d ",c_array[a]);
	     fprintf (fp, "\n");
	   }
	 return array;
	 
       }
     else
       {
	 for (a=max; a<=min;a+=step)
	   {
	     c_array[f]=a;
	     r_generate_array_int_list (len, min, max, step, array, f+1, n,fp, c_array);
	   }
       }
     return array;
   }

char *** r_generate_array_string_list ( int len, char ***alp,int *alp_size, char ***array, int f, int *n,FILE *fp, char **c_array, int mode, int pstart);
char ***generate_array_string_list (int len, char ***alp, int *alp_size, int *n, char *file, int mode)
   {
     char  ***array, **c_array;
     FILE *fp=NULL;
     
     if (file!=NULL)
       {
	 array=NULL;
	 n[0]=0;
	 fp=vfopen (file, "w");
       }
     else
       {
	 
	 int a,s;
	 n[0]=0;
	 for (s=1, a=0; a<len; a++)
	   {
	     s*=alp_size[a];
	   }
	 array=declare_arrayN (3,sizeof (char), s, len,0);
       }
     c_array=declare_char (len,0);
     array=r_generate_array_string_list ( len, alp, alp_size, array, 0, n,fp, c_array, mode, -1);
     vfree (c_array);
     if ( fp) vfclose (fp);
     return array;
   }
char *** r_generate_array_string_list ( int len, char ***alp, int *alp_size, char  ***array, int f, int *n,FILE *fp, char **c_array, int mode, int pstart)
   {
     int a;
     int start;
     
     if ( f==len)
       {
	 if ( array)
	   {
	     for (a=0; a<len; a++)
	       {
		 array[n[0]][a]=c_array[a];
	       }
	     n[0]++;
	   }
	 else
	   {
	     
	     n[0]++;
	     for (a=0; a<len; a++)fprintf ( fp, "%s ",c_array[a]);
	     fprintf (fp, "\n");
	   }
	 return array;
	 
       }
     else
       {
	 if ( mode==OVERLAP)
	   {
	     start=0;
	   }
	 else if ( mode==NO_OVERLAP) 
	   {
	     start=pstart+1;
	   }

	 for (a=start; a<alp_size[f]; a++)
	   {
	     c_array[f]=alp[f][a];
	     r_generate_array_string_list (len,alp, alp_size, array, f+1, n,fp, c_array, mode, a);
	   }
       }
     return array;
   }
float *display_accuracy (float *count, FILE *fp)
{
  float *r;
  r=counts2accuracy (count);
  fprintf (fp, "Sp: %.3f Sn: %.3f Sen2: %.3f AC: %.3f\n",r[0], r[1], r[2], r[3]);
  vfree (r);
  return count;
}
float *counts2accuracy (float *count)
{
  //0: TP
  //1: TN
  //2: FP
  //3: FN
  float *result;
  float TP, TN, FP, FN;
  
  result=vcalloc (4, sizeof (float));
  TP=count[0];
  TN=count[1];
  FP=count[2];
  FN=count[3];
  
  
  result [0]=((TN+FP)==0)?-1:TN/(TN+FP); //Sp
  result [1]=((TP+FN)==0)?-1:TP/(TP+FN); //Sn
  result [2]=((TP+FP)==0)?-1:TP/(TP+FP); //Sen2
  result [3]=(((TP+FN)==0) || ((TP+FP)==0) ||  ((TN+FP)==0) || ((TN+FN)==0))?-1:0.5*((TP/(TP+FN)) + (TP/(TP+FP)) + (TN/(TN+FP)) + (TN/(TN+FN))) - 1 ;//AC
  
  return result;
}

float  rates2sensitivity (int tp, int tn, int fp, int fn, float *sp, float *sn, float *sen2, float *b)
{
  if (sp==NULL)
    {
      sp=vcalloc (1, sizeof (float));
      sn=vcalloc (1, sizeof (float));
      sen2=vcalloc (1, sizeof (float));
      b=vcalloc (1, sizeof (float));
    }
  sn[0]  =((tp+fn)==0)?1:(float)tp/(float)(tp+fn);
  sen2[0]=((tp+fp)==0)?1:(float)tp/(float)(tp+fp);
  sp[0]  =((tn+fp)==0)?1:(float)tn/(float)(tn+fp);
  b[0]=MIN((MIN((sp[0]),(sn[0]))),(sen2[0]));
  return b[0];
}
float profile2sensitivity (char *pred, char *ref, float *sp, float *sn, float *sen2, float *b)	      
{
  int tp=0, tn=0, fp=0, fn=0;
  int a, l;
  
  l=strlen (pred);
  
  for (a=0; a<l; a++)
    {
      if (pred[a]=='I' && ref[a]=='I')tp++;
      if (pred[a]=='O' && ref[a]=='I')fn++;
      if (pred[a]=='I' && ref[a]=='O')fp++;
      if (pred[a]=='O' && ref[a]=='O')tn++;
    }
  return  rates2sensitivity (tp, tn, fp, fn, sp, sn, sen2, b);
}

float profile2evalue (char *pred, char *ref)
{
  
  int a, l;
  double P=0;
  double E=0;
  double II=0;
  double p1, p2, p;
  l=strlen (pred);
  
  for (a=0; a<l; a++)
    {
      if (pred[a]=='I')P++;
      if (ref[a]=='I') E++;
      
      if (pred[a]=='I' && ref[a]=='I')II++;
    }

  if (II==0)return 0;

  p1= M_chooses_Nlog (P,l) + M_chooses_Nlog (II, P) + M_chooses_Nlog (E-II, l-P);
  p2=(M_chooses_Nlog (P,l)+M_chooses_Nlog (E,l));
  p=(p1-p2);

  return (float)-p;
}






		      
// NEW Intitialization

int string_putenv    (char *p);
char *env_file;

char ** standard_initialisation  (char **in_argv, int *in_argc)
{
  
  char *s;
  static int done=0;
  char buf[1000];
  char **out_argv;
  int a, stdi,c;
  FILE *fp;
  char *env_file;
  char parent[100];
  

 
  //Debug things
  debug_lock=atoigetenv("DEBUG_LOCK");
  
 

  if (!in_argv){done=0;return NULL;}
  else if ( done){return in_argv;}
  else done=1;


  get_time();	
  
/*Standard exit*/
  global_exit_signal=EXIT_SUCCESS;
  atexit (clean_exit);	
  

  
  signal (SIGTERM,signal_exit);
  signal (SIGINT, signal_exit);
  signal (SIGKILL, signal_exit);
  
  signal (SIGABRT ,error_exit);
  signal (SIGFPE  ,error_exit);
  signal (SIGILL  ,error_exit);
  signal (SIGSEGV ,error_exit);
  
  program_name=vcalloc ( strlen (in_argv[0])+strlen (PROGRAM)+1, sizeof (char));
  if (in_argv)
    {
      sprintf ( program_name, "%s", in_argv[0]);
      out_argv=break_list ( in_argv, in_argc, "=;, \n");
      s=list2string2 (out_argv, in_argc[0], " ");
      set_command_line (s);
    }
  else sprintf ( program_name, "%s",PROGRAM); 
  
  if ( name_is_in_list ( "-no_error_report", out_argv, in_argc[0], 100)!=-1)no_error_report=1;
  
  //Plugins
  
  get_os();
  get_nproc();
	
  

  if (!getenv ("UPDATED_ENV_4_TCOFFEE"))
    {
      //1-set the envirnement variables;
      file_putenv ("/usr/local/t_coffee/.t_coffee_env");//make sure child processes do not upate env
      sprintf (buf, "%s/.t_coffee/.t_coffee_env", getenv ("HOME"));
      file_putenv (buf);
      file_putenv ("./.t_coffee_env");
      if (getenv ("ENV_4_TCOFFEE"))file_putenv (getenv ("ENV_4_TCOFFEE"));
    }
  
  string_putenv (s); //let Command line update go through
  
  
  cputenv ("HOME_4_TCOFFEE=%s",get_home_4_tcoffee());
  cputenv ("DIR_4_TCOFFEE=%s",get_dir_4_tcoffee());
  cputenv ("TMP_4_TCOFFEE=%s",get_tmp_4_tcoffee());
  cputenv ("CACHE_4_TCOFFEE=%s",get_cache_4_tcoffee());
  cputenv ("MCOFFEE_4_TCOFFEE=%s",get_mcoffee_4_tcoffee());
  cputenv ("METHODS_4_TCOFFEE=%s",get_methods_4_tcoffee());
  cputenv ("PLUGINS_4_TCOFFEE=%s",get_plugins_4_tcoffee());
  cputenv ("LOCKDIR_4_TCOFFEE=%s",get_lockdir_4_tcoffee());
  cputenv ("ERRORFILE_4_TCOFFEE=t_coffee.ErrorReport");
  
  
  
  
  
  string_putenv (s); //let Command line update go through //Twice in case an executable dir not created
  
  if (!getenv ("ENV_4_TCOFFEE"))cputenv ("ENV_4_TCOFFEE=%s/.t_coffee_env", get_dir_4_tcoffee());
  
  
  if (!getenv ("UPDATED_ENV_4_TCOFFEE"))
    {
      cputenv4path ("/usr/local/t_coffee/plugins");
      sprintf (buf, "%s/.t_coffee/plugins", getenv ("HOME"));
      cputenv4path (buf);
      cputenv4path ("./.plugins");
      if ( getenv ("PLUGINS_4_TCOFFEE"))cputenv4path (getenv ("PLUGINS_4_TCOFFEE"));
    }
  cputenv ("UPDATED_ENV_4_TCOFFEE=1");
 
  if ( debug_lock){fprintf ( stderr, "\n*************** LOCKDIR: %s *************\n", get_lockdir_4_tcoffee());}
    
  lock(getpid(),LLOCK, LRESET, "%d\n",getppid());//set the main lock
  if (is_shellpid(getppid()))lock(getppid(),LLOCK, LSET, "%d\n",getpid());//update parent lock when parent is shell
      
  
  //set special Variables
  if (getenv ("MAFFT_BINARIES") || isdir4path ("/usr/local/lib/mafft"));
  else cputenv ( "MAFFT_BINARIES=%s",get_plugins_4_tcoffee());
  
  //set proxy
  set_proxy(get_proxy_from_env());
  set_email(get_email_from_env ());
    
  //pipe_in
  for (a=1, stdi=0; a<in_argc[0]; a++)
	{
	  if ( (strm ( out_argv[a], "stdin") || strm (out_argv[a], "STDIN")) && stdi==0)
	    {
	      char *file;
	      FILE *fp, *fp_stdi;
	      
	      if ( stdi==0)
		{
		  file=vtmpnam (NULL);
		  vfree (out_argv[a]);
		  out_argv[a]=vcalloc ( strlen (file)+1, sizeof (char));
		  sprintf (out_argv[a], "%s", file);
		  fp_stdi=vfopen ("stdin", "r");
		  fp=vfopen (file, "w");
		  while ( (c=fgetc (fp_stdi))!=EOF)fprintf (fp, "%c", c);
		  vfclose (fp);
		  vfclose (fp_stdi);
		  stdi=1;
		}
	      else if ( stdi ==1)
		{
		  printf_exit ( EXIT_FAILURE,stderr, "ERROR: Attempt to get more than one file via pipe [FATAL:%s]\n", PROGRAM);
		}
	    }
	}
  
  return out_argv;
}
void signal_exit ()
 {
   
   if (is_rootpid())fprintf ( stderr, "****** Forced interuption of main parent process %d\n", getpid());
   
   global_exit_signal=EXIT_SUCCESS;
   myexit (EXIT_SUCCESS);
 }
void error_exit ()
 {
   lock (getpid(), LERROR, LSET, "%d -- ERROR: COREDUMP: %s %s\n",getpid(), PROGRAM, VERSION, getpid());
   global_exit_signal=EXIT_FAILURE;
   myexit (EXIT_FAILURE);
 }
void clean_exit ()
{
  Tmpname *b;
  char *tmp;
  static int done;
  Tmpname *start;
  int debug;

  clean_exit_started=1;//prevent new locks 
  
  start=tmpname;
  //1-start killing all the children
  
  
  //3-update error lock if needed
  if (has_error_lock())//Update error lock
    {
      lock (getpid(), LERROR, LSET, "%d -- STACK: %d -> %d -- %s %s\n", getpid(), getppid(), getpid(), PROGRAM, VERSION);
      lock (getpid(), LERROR, LSET, "%d -- COM: %s\n",getpid(),in_cl );
      
      //
    }
    
  if (is_rootpid())
    {
      kill_child_pid(getpid());
      if (has_error_lock())
	{
	  char *e=NULL;
	  stack_msg (stderr);
	  warning_msg (stderr);
	  e=lock (getpid(), LERROR, LREAD, NULL);
	  
	  
	  	  	  
	  //explicit the most common error messages
	  if ( strstr (e, "EMAIL"))email_msg (stderr);
	  if ( strstr (e, "INTERNET"))proxy_msg (stderr);
	  if ( strstr (e, "PG"))   install_msg(stderr);
	  if ( strstr (e, "COREDUMP"))
	    {
	      error_msg (stderr);
	      dump_error_file();
	    }
	  print_exit_failure_message ();
	  vfree (e);
	}
      else if ( has_warning_lock())
	{
	   warning_msg (stderr);
	}
      else
	print_exit_success_message();

      lock (getpid(), LLOCK, LRELEASE, "");
      lock (getpid(), LWARNING, LRELEASE, "");
      lock (getpid(), LERROR, LRELEASE, "");
    }

  
  
  
  add_method_output2method_log (NULL, NULL, NULL, NULL, decode_name (NULL, CODELIST));
  
  //Remove all temporary files
  debug=(atoigetenv ("DEBUG_TMP_FILE"));

  
  
  while ( start)
    {
      if (!debug)
	{
	  
	  if (isdir(start->name))rrmdir (start->name);
	  else 
	    {
	      char test[1000];
	      vremove (start->name);
	      if (start->name)sprintf (test, "%s.dnd", start->name);vremove (test);
	      if (start->name)sprintf (test, "%s.html",start->name);vremove (test);
	    }
	}
      else
	{
	  if (isdir(start->name))
	    {fprintf ( stderr, "DEBUG_TMP_FILE SET : Dir  %s not removed (%d)\n", start->name, getpid());}
	  else
	    {fprintf ( stderr, "DEBUG_TMP_FILE SET : File %s not removed (%d)\n", start->name, getpid());}
	}
      b=start;
      start=start->next;
      //vfree(b->name);vfree(b);
    }
  
  //Remove the lock 
  //lock (getpid(), LLOCK, LRELEASE,NULL); Now keep the lock unless it is a parent process
  
  //UNIQUE TERMINATION FOR EVERYBODY!!!!!!
  return;
}

    
int cputenv4path (char *p)
{
  if (!p)return 0;
  else if (isdir4path (p))
    {
      cputenv ("PATH=%s:%s", p, getenv("PATH"));
      return 1;
    }
  else
    {
      return 0;
    }
}
      

int string_putenv ( char *s)
{
  //extract from command line all the occurences -setenv val1 val2 and sets environement
  
  char *p;
  int n;
  char *v1, *v2;

  
  if (!s) return 0;
  v1=vcalloc ( strlen (s)+1, sizeof (char));
  v2=vcalloc ( strlen (s)+1, sizeof (char));
  
  p=s;
  n=0;
  while ( p=strstr (p, "-setenv"))
    {
      if (sscanf (p, "-setenv %s %s", v1,v2)==2)
	{
	  
	  if (strm (v1, "PATH"))cputenv4path (v2);
	  else cputenv ( "%s=%s", v1, v2);
	}
       p+=strlen ("-setenv");
       n++;
    }
  p=s;
  if ( (p=strstr (p, "-plugins")))
    {
      sscanf (p, "-plugins %s", v1);
      cputenv ("PROXY_4_TCOFFEE=%s",v1);
      cputenv4path (v1);
      
    }
  p=s;
  if ( (p=strstr (p, "-email")))
    {
      sscanf (p, "-email %s", v1);
      cputenv ("EMAIL_4_TCOFFEE=%s", v1);
    }
  p=s;
  if ( (p=strstr (p, "-proxy")))
    {
      sscanf (p, "-proxy %s", v1);
      cputenv ("PROXY_4_TCOFFEE=%s", v1);
    }
  
       
  
  vfree (v1); vfree (v2);
  return n;
}

char* file_putenv (char *file)
{
  //puts in environement all the variables conatinned in file
  //format VAR=value on each line
  
  char ***list;
  int n=0;
  
  
  if (!file || !file_exists(NULL,file)) return NULL;
  
  list=file2list (file, "\n=");
  fprintf ( stderr, "Import Environement Variables from %s\n", file);
  
  while (list[n])
    {
      if ( list[n][1][0]!='#')
	{
	  if ( strm (list[n][1], "PATH"))
	    {
	      
	      cputenv ( "PATH=%s:%s",list[n][2], getenv ("PATH")); 
	      fprintf ( stderr, "\tPATH=%s:$PATH", list[n][2]);
	    }
	  else
	    {
	      
	      cputenv("%s=%s", list[n][1],list[n][2]);
	      fprintf ( stderr, "\t%s=%s",  list[n][1],list[n][2]);
	    }
	  n++;
	}
    }
  free_arrayN ((void ***)list, 3);
  return NULL;
 }

char * bachup_env (char *mode,char *f)
{
  static char *file;
  static char *buf;
  if (!file)
    {
      file=vtmpnam (NULL);
      buf=vcalloc ( 10000, sizeof (char));
    }
  
  if (!f)f=file;
  if (strm (mode, "DUMP"))
    {
      printf_system_direct ("/usr/bin/env > %s", f);
      return EXIT_SUCCESS;
    }
  else if ( strm (mode,"RESTAURE") && file_exists (NULL,f))
    {
      file_putenv (f);
      return EXIT_SUCCESS;
    }
  return NULL;
}
  
 