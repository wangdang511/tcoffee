#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#include "io_lib_header.h"
#include "util_lib_header.h"
#include "define_header.h"
#include "dp_lib_header.h"

int count_threshold_nodes (Alignment *A, NT_node P, int t);
int set_node_score (Alignment *A, NT_node P, char *mode);
char *split_nodes_nseq (Alignment *A, NT_node P, int max_nseq, char *list);
char *split_nodes_idmax (Alignment *A, NT_node P, int max_id, char *list);
/******************************************************************/
/*                   MAIN DRIVER                                  */
/*                                                                */
/*                                                                */
/******************************************************************/
int method_uses_structure(TC_method *M)
{
  if ( strchr (M->seq_type, 'P'))return 1;
  else return 0;
}
Constraint_list *profile2list     (Job_TC *job, int nprf)
{
  int a,b, s1, s2, r1, r2,w2;
  Constraint_list *CL, *RCL;
  Sequence *S;
  Alignment *A1, *A2;
  int *iA1, *iA2;
  TC_method *M;
  int ***RI, ***NRI;
  Sequence *RS;
  int Rne;
  
  int *seqlist;
  int **cache;
  static int *entry;

  
  if (!entry)entry=vcalloc ( ICHUNK+3, sizeof (int));
  
  CL=(job->io)->CL;
  //1- buffer CL
  
  RS=CL->S;
  RI=CL->residue_index;
  Rne=CL->ne;
  M=(job->param)->TCM;
  
  
   
  
  
  
  //Index
  seqlist=string2num_list ((job->param)->seq_c)+1;
  A1=seq2profile(CL->S, seqlist[1]);
  A2=seq2profile(CL->S, seqlist[2]);
   
  S=merge_seq (A1->S, NULL  );
  S=merge_seq (A2->S, S);
      
  iA1=get_name_index (A1->name,A1->nseq, S->name, S->nseq);
  iA2=get_name_index (A2->name,A2->nseq, S->name, S->nseq);
  cache=vcalloc ( S->nseq, sizeof (int*));
  for (a=0; a<A1->nseq; a++)cache[iA1[a]]=seq2inv_pos (A1->seq_al[a]);
  for (a=0; a<A2->nseq; a++)cache[iA2[a]]=seq2inv_pos (A2->seq_al[a]);

  //Declare local CL
  CL->S=S;
  CL->residue_index=declare_residue_index(S);
  CL->ne=0;
  
  //Compute lib
   for (a=0; a<A1->nseq; a++)
     for ( b=0; b<A2->nseq; b++)
       {
	 char buf[1000];
	 sprintf ( buf, "2 %d %d",iA1[a], iA2[b]);
	 job=print_lib_job (job, "param->seq_c=%s", buf);
	 CL=seq2list (job);
       }
   
   //restaure CL;
   CL->S=RS;
   NRI=CL->residue_index;
   CL->residue_index=RI;
   CL->ne=Rne;
   

  
   //incorporate new lib
   entry[SEQ1]=seqlist[1];
   entry[SEQ2]=seqlist[2];
   for (a=0; a<A1->nseq; a++)
     {
       s1=iA1[a];
       
       for (r1=1; r1<=S->len[s1];r1++)
	 {
	   for (b=1; b<NRI[s1][r1][0]; b+=ICHUNK)
	     {
	       int s2=NRI[s1][r1][b+SEQ2];
	       int r2=NRI[s1][r1][b+R2];
	       int w2=NRI[s1][r1][b+WE];
	       
	       entry[R1]=cache[s1][r1];
	       entry[R2]=cache[s2][r2];
	       entry[WE]=w2;
	       entry[CONS]=1;
	       entry[MISC]=1;
	       add_entry2list (entry,CL);
	     }
	 }
     }
   
   free_int (cache, -1);
   free_arrayN (NRI, 3);
   vfree (entry);
   free_sequence (S, S->nseq);
   return (job->io)->CL=CL;
}

Constraint_list *seq2list     ( Job_TC *job)
    {
      char *mode;
      Alignment *A=NULL;
      Constraint_list *PW_CL;
      Constraint_list *RCL=NULL;

      int full_prf, nprf;
      int *seqlist;




      static char *s1, *s2;
      Sequence *S, *STL;
      Constraint_list *CL;


      char *seq;
      char *weight;
      TC_method *M;


      M=(job->param)->TCM;
      mode=M->executable;
      weight=M->weight;

      PW_CL=M->PW_CL;

      CL=(job->io)->CL;
      seq=(job->param)->seq_c;



      S=(CL)?CL->S:NULL;
      STL=(CL)?CL->STRUC_LIST:NULL;




      seqlist=string2num_list (seq)+1;

      if (!s1)s1=vcalloc ( MAXNAMES+1, sizeof (char));
      if (!s2)s2=vcalloc ( MAXNAMES+1, sizeof (char));


      sprintf (s1, "%s", (CL->S)->name[seqlist[1]]);
      sprintf (s2, "%s", (CL->S)->name[seqlist[2]]);

/*Proteins*/


      if ( strncmp (CL->profile_comparison, "full", 4)==0)
	     {
	       full_prf=1;
	       if ( CL->profile_comparison[4])nprf=atoi ( CL->profile_comparison+4);
	       else
		 nprf=0;
	     }
      else
	     {
	       full_prf=0;
	     }

      if ((method_uses_structure (M)) && profile2P_template_file (CL->S, seqlist[1]) && profile2P_template_file (CL->S, seqlist[2]))
	{
	  RCL=profile2list     (job, nprf);
	}
      else if ( strm (mode, "ktup_msa"))
	{
	  RCL=hasch2constraint_list (CL->S, CL);
	}
      else if (     strm (mode, "test_pair") || strm ( mode,"fast_pair")         || strm (mode, "ifast_pair") \
		   || strm ( mode, "diag_fast_pair")|| strm (mode, "idiag_fast_pair")\
		   || strm ( mode, "blast_pair")    || strm (mode, "lalign_blast_pair") \
		   || strm ( mode, "viterbi_pair")  || strm (mode, "slow_pair")      || strm(mode, "glocal_pair") || strm (mode, "biphasic_pair") \
		   || strm ( mode, "islow_pair")    || strm (mode, "tm_slow_pair") || strm (mode, "r_slow_pair") \
		   || strm ( mode, "lalign_id_pair")|| strm (mode, "tm_lalign_id_pair") || strm (mode , "lalign_len_pair") \
		   || strm (mode, "prrp_aln")       || strm ( mode, "test_pair") \
		   || strm (mode, "cdna_fast_pair") || strm (mode, "diaa_slow_pair") || strm (mode, "monoaa_slow_pair")\
		   || strncmp (mode,"cdna_fast_pair",14)==0	\
		   )
	{

	  A=fast_pair (job);
	  RCL=aln2constraint_list ((A->A)?A->A:A, CL,weight);
	}

      else if ( strm ( mode, "subop1_pair") || strm ( mode, "subop2_pair") )
	{
	  A=fast_pair (job);
	  RCL=A->CL;

	}
      else if ( strm ( mode, "proba_pair") )
	{
	  A=fast_pair (job);
	  RCL=A->CL;
	}
      else if ( strm ( mode, "best_pair4prot"))
	{
	  RCL=best_pair4prot (job);
	}
	else if ( strm ( mode, "best_pair4rna"))
	{
	  RCL=best_pair4rna (job);
	}
      else if ( strm ( mode, "exon2_pair"))
	{

	  char weight2[1000];

	  A=fast_pair (job);
	  sprintf ( weight2, "%s_subset_objOBJ-",weight);
	  RCL=aln2constraint_list (A, CL,weight2);
	}
      else if ( strm ( mode, "exon_pair"))
	{

	  A=fast_pair (job);
	  RCL=aln2constraint_list (A, CL,weight);

	}
      else if ( strm ( mode, "exon3_pair"))
	{

	  char weight2[1000];

	  A=fast_pair (job);
	  sprintf ( weight2, "%s_subset_objOBJ-",weight);
	  RCL=aln2constraint_list (A, CL,weight2);
	}

/*STRUCTURAL METHODS*/

       else if ( strm (mode, "seq_msa"))
	{
	  RCL=seq_msa(M, seq, CL);
	}
/*STRUCTURAL METHODS*/
      else if (strm (mode, "profile_pair") || strm (mode, "hh_pair"))
	{
	  RCL=profile_pair (M, seq, CL);
	}

      else if ( strm (mode, "sap_pair"))
	{
	  RCL=sap_pair (seq, weight, CL);
	}
      else if ( strm (mode, "thread_pair"))
	{
	  RCL=thread_pair (M,seq, CL);
	}
      else if ( strm (mode, "pdb_pair"))
	{
	  RCL=pdb_pair (M,seq, CL);
	}
	else if (strm (mode, "rna_pair"))
	{
		RCL=rna_pair(M, seq, CL);
	}
      else if ( strm (mode, "pdbid_pair"))
	{
	  RCL=pdbid_pair (M,seq, CL);
	}
      else if ( strm (mode, "fugue_pair"))
	{
	  RCL=thread_pair (M,seq, CL);
	}
      else if ( strm (mode, "lsqman_pair"))
	{
	  RCL=lsqman_pair(seq, CL);
	}
      else if ( strm ( mode, "align_pdb_pair"))
	{
	  RCL=align_pdb_pair ( seq,"gotoh_pair_wise", CL->align_pdb_hasch_mode,CL->align_pdb_param_file,CL, job);
	}
      else if ( strm ( mode, "lalign_pdb_pair"))
	{
	  RCL=align_pdb_pair ( seq,"sim_pair_wise_lalign", CL->align_pdb_hasch_mode,CL->align_pdb_param_file,CL, job);
	}
      else if ( strm ( mode, "align_pdb_pair_2"))
	{
	  RCL=align_pdb_pair_2 ( seq, CL);
	}
      else
	{
	  fprintf ( CL->local_stderr, "\nERROR: THE FUNCTION %s DOES NOT EXIST [FATAL:%s]\n", mode, PROGRAM);crash("");
	}
      add_method_output2method_log (NULL,NULL, (A&&A->len_aln)?A:NULL,RCL, NULL);
      RCL=(RCL==NULL)?CL:RCL;


      vfree ( seqlist-1);
      free_aln (A);
      return RCL;
    }

Constraint_list *method2pw_cl (TC_method *M, Constraint_list *CL)
    {
      char *mode;
      Constraint_list *PW_CL=NULL;
      Sequence *S;
      char mat[100], *m;
      char group_mat[100];



      mode=M->executable;
      PW_CL=copy_constraint_list ( CL, SOFT_COPY);
      PW_CL->pw_parameters_set=1;



      S=(PW_CL)?PW_CL->S:NULL;

      /*DNA or Protein*/
      m=PW_CL->method_matrix;
      if (  strm ((PW_CL->S)->type, "PROTEIN"))
	   {

	     sprintf ( mat, "%s", (strm(m, "default"))?"blosum62mt":m);
	     sprintf (group_mat, "vasiliky");
	     PW_CL->ktup=2;

	   }
      else if (  strm ((PW_CL->S)->type, "DNA") || strm ((PW_CL->S)->type, "RNA") )
	   {

	      sprintf(group_mat, "idmat");
	      sprintf ( mat, "%s", (strm(m, "default"))?"dna_idmat":m);
	      PW_CL->ktup=5;

	   }
      if ( M->matrix[0])sprintf ( mat, "%s", M->matrix);

      PW_CL->M=read_matrice (mat);


      if ( M->gop!=UNDEFINED) {PW_CL->gop=M->gop;}
      else
	{
	  PW_CL->gop= get_avg_matrix_mm (PW_CL->M, AA_ALPHABET)*10;
	}

      if ( M->gep!=UNDEFINED)PW_CL->gep=M->gep;
      else PW_CL->gep=-1;



      if ( strm2 ( mode,"fast_pair", "ifast_pair"))
	    {
	      PW_CL->maximise=1;
              PW_CL->TG_MODE=1;
              PW_CL->use_fragments=0;
              if ( !PW_CL->use_fragments)PW_CL->diagonal_threshold=0;
              else PW_CL->diagonal_threshold=6;

              sprintf (PW_CL->dp_mode, "fasta_pair_wise");
      	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);

              if ( strm ( mode, "fast_pair"))
                {
		  PW_CL->residue_index=NULL;

                  PW_CL->get_dp_cost=slow_get_dp_cost;
                  PW_CL->evaluate_residue_pair=evaluate_matrix_score;
                  PW_CL->extend_jit=0;
		}
	    }
	else if ( strm2 ( mode,"diag_fast_pair","idiag_fast_pair"))
	    {
	      PW_CL->residue_index=NULL;
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->S=CL->S;

	      PW_CL->use_fragments=1;
	      PW_CL->diagonal_threshold=3;

	      sprintf (PW_CL->dp_mode, "fasta_pair_wise");
	      PW_CL->ktup=1;
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);

	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;

	      PW_CL->extend_jit=0;
	    }
	else if ( strm ( mode,"blast_pair"))
	    {
	      PW_CL->residue_index=NULL;
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;

	      PW_CL->use_fragments=0;

	      PW_CL->pair_wise=gotoh_pair_wise;
	      PW_CL->evaluate_residue_pair=evaluate_blast_profile_score;
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->extend_jit=0;
	    }
	else if ( strm ( mode,"lalign_blast_pair"))
	    {
	      PW_CL->residue_index=NULL;
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;

	      PW_CL->use_fragments=0;
	      PW_CL->pair_wise=sim_pair_wise_lalign;
	      PW_CL->evaluate_residue_pair=evaluate_blast_profile_score;
	      PW_CL->lalign_n_top=10;

	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->extend_jit=0;
	    }
	else if ( strm ( mode,"viterbi_pair"))
	    {
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "viterbi_pair_wise");
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	      PW_CL->extend_jit=0;
	    }
	else if ( strm ( mode,"glocal_pair"))
	    {
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "glocal_pair_wise");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);

	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	      PW_CL->extend_jit=0;
	    }
        else if ( strm ( mode,"test_pair"))
	    {
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "test_pair_wise");
	    }
        else if ( strm ( mode,"sticky_pair"))
	  {
	    PW_CL->maximise=1;
	    PW_CL->TG_MODE=1;
	    PW_CL->use_fragments=0;
	    PW_CL->residue_index=NULL;
	    PW_CL->get_dp_cost=cw_profile_get_dp_cost;
	    PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	    PW_CL->extend_jit=0;
	    sprintf (PW_CL->dp_mode, "gotoh_pair_wise_lgp_sticky");
	    sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	  }

	else if ( strm ( mode,"slow_pair")|| strm (mode, "islow_pair" ) )
	    {

	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "myers_miller_pair_wise");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);

	      if ( strm ( "islow_pair", mode))
		{
		  PW_CL->get_dp_cost=slow_get_dp_cost;
		  PW_CL->evaluate_residue_pair=residue_pair_extended_list;
		  PW_CL->extend_jit=1;
		}
	      else if ( strm ("slow_pair", mode) )
		{
		  PW_CL->residue_index=NULL;
		  PW_CL->get_dp_cost=cw_profile_get_dp_cost;
		  PW_CL->evaluate_residue_pair=evaluate_matrix_score;
		  PW_CL->extend_jit=0;
		}
	    }
      	else if ( strm (mode, "subop1_pair"))
	    {

	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "subop1_pair_wise");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	      PW_CL->extend_jit=0;
	    }
       else if ( strm (mode, "biphasic_pair"))
	    {
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "biphasic_pair_wise");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=cw_profile_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	      PW_CL->extend_jit=0;
	    }
      else if ( strm (mode, "proba_pair"))
	    {

	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "proba_pair_wise");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	      PW_CL->extend_jit=0;
	    }

      	else if ( strm (mode, "diaa_slow_pair"))
	    {

	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "gotoh_pair_wise_lgp");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_diaa_matrix_score;
	      PW_CL->extend_jit=0;
	    }
      	else if ( strm (mode, "r_slow_pair"))
	    {
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "gotoh_pair_wise_lgp");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	      PW_CL->extend_jit=0;
	      PW_CL->reverse_seq=1;
	    }
	else if ( strm (mode, "tm_slow_pair"))
	    {
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "myers_miller_pair_wise");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_tm_matrix_score;
	      PW_CL->extend_jit=0;
	    }
	else if ( strm (mode, "monoaa_slow_pair"))
	    {

	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "gotoh_pair_wise_lgp");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_monoaa_matrix_score;
	      PW_CL->extend_jit=0;
	    }
	else if ( strm (mode, "subop2_pair"))
	  {

	    PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "subop2_pair_wise");
	      sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	      PW_CL->residue_index=NULL;
	      PW_CL->get_dp_cost=slow_get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	      PW_CL->extend_jit=0;
	    }

     	else if (strm ( mode, "exon2_pair"))
	  {
	    int a;
	    PW_CL->maximise=1;
	    PW_CL->TG_MODE=1;

	    PW_CL->use_fragments=0;
	    sprintf (PW_CL->dp_mode, "myers_miller_pair_wise");
	    sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	    PW_CL->residue_index=NULL;

	    for ( a=0; a<60; a++)
	      {
		PW_CL->M['x'-'A'][a]=0;
		PW_CL->M[a]['x'-'A']=0;
		PW_CL->M['X'-'A'][a]=0;
		PW_CL->M[a]['X'-'A']=0;
	      }
	    PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	    PW_CL->extend_jit=0;
	  }
      	else if (strm ( mode, "exon3_pair"))
	  {
	    int a;
	    PW_CL->maximise=1;
	    PW_CL->TG_MODE=1;

	    PW_CL->use_fragments=0;
	    sprintf (PW_CL->dp_mode, "myers_miller_pair_wise");
	    sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	    PW_CL->residue_index=NULL;

	    for ( a=0; a<60; a++)
	      {
		PW_CL->M['x'-'A'][a]=0;
		PW_CL->M[a]['x'-'A']=0;
		PW_CL->M['X'-'A'][a]=0;
		PW_CL->M[a]['X'-'A']=0;
	      }
	    PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	    PW_CL->extend_jit=0;
	  }
	else if (strm ( mode, "exon_pair"))
	  {
	    int a;
	    PW_CL->maximise=1;
	    PW_CL->TG_MODE=1;

	    PW_CL->use_fragments=0;
	    sprintf (PW_CL->dp_mode, "myers_miller_pair_wise");
	    sprintf (PW_CL->matrix_for_aa_group, "%s",group_mat);
	    PW_CL->residue_index=NULL;

	    for ( a=0; a<60; a++)
	      {
		PW_CL->M['x'-'A'][a]=0;
		PW_CL->M[a]['x'-'A']=0;
		PW_CL->M['X'-'A'][a]=0;
		PW_CL->M[a]['X'-'A']=0;
	      }
	    PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	    PW_CL->extend_jit=0;
	  }
	else if ( strm ( mode , "lalign_len_pair"))
	  {
	    PW_CL->residue_index=NULL;
	    PW_CL->maximise=1;
	    PW_CL->TG_MODE=1;
	    PW_CL->use_fragments=0;
	    PW_CL->pair_wise=sim_pair_wise_lalign;
	    PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	    PW_CL->get_dp_cost=slow_get_dp_cost;
	    PW_CL->lalign_n_top=CL->lalign_n_top;
	    sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	    PW_CL->extend_jit=0;
	  }
	else if ( strm ( mode , "lalign_id_pair"))
	  {
	    PW_CL->residue_index=NULL;
	    PW_CL->maximise=1;
	    PW_CL->TG_MODE=1;
	    PW_CL->use_fragments=0;
	    PW_CL->pair_wise=sim_pair_wise_lalign;
	    PW_CL->evaluate_residue_pair=evaluate_matrix_score;
	    PW_CL->get_dp_cost=slow_get_dp_cost;
	    PW_CL->lalign_n_top=CL->lalign_n_top;
	    sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	    PW_CL->extend_jit=0;
	  }
      	else if ( strm ( mode , "tm_lalign_id_pair"))
	  {
	    PW_CL->residue_index=NULL;
	    PW_CL->maximise=1;
	    PW_CL->TG_MODE=1;
	    PW_CL->use_fragments=0;
	    PW_CL->pair_wise=sim_pair_wise_lalign;
	    PW_CL->evaluate_residue_pair=evaluate_tm_matrix_score;
	    PW_CL->get_dp_cost=slow_get_dp_cost;
	    PW_CL->lalign_n_top=CL->lalign_n_top;
	    sprintf (PW_CL->matrix_for_aa_group,"%s", group_mat);
	    PW_CL->extend_jit=0;
	  }
/*CDNA*/
	else if ( strm ( mode, "cdna_cfast_pair"))
	    {
	      PW_CL->residue_index=NULL;
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->S=CL->S;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "cfasta_cdna_pair_wise");

	      PW_CL->M=read_matrice (strcpy ( mat, "blosum62mt"));
	      PW_CL->extend_jit=0;

	      PW_CL->f_gop=CL->f_gop;
	      PW_CL->f_gep=CL->f_gep;
	      PW_CL->get_dp_cost=get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_cdna_matrix_score;
	      PW_CL->ktup=1;
	    }
	else if ( strm ( mode, "cdna_fast_pair") ||  strncmp (mode,"cdna_fast_pair",14)==0)
	    {

	      PW_CL->residue_index=NULL;
	      PW_CL->maximise=1;
	      PW_CL->TG_MODE=1;
	      PW_CL->use_fragments=0;
	      sprintf (PW_CL->dp_mode, "fasta_cdna_pair_wise");

	      PW_CL->extend_jit=0;
	      PW_CL->gop=-5;
	      PW_CL->gep=-1;
	      PW_CL->f_gop=-15;
	      PW_CL->f_gep=0;

	      PW_CL->get_dp_cost=get_dp_cost;
	      PW_CL->evaluate_residue_pair=evaluate_cdna_matrix_score;
	      PW_CL->ktup=1;
	    }
	else
	  {
	    free_constraint_list (PW_CL);
	    PW_CL=NULL;
	  }


      if (!strm (CL->method_evaluate_mode, "default"))
	{
	  choose_extension_mode (CL->method_evaluate_mode, PW_CL);
	}
      return PW_CL;
    }
/******************************************************************/
/*                   MULTIPLE ALIGNMENTS                          */
/*                                                                */
/*                                                                */
/******************************************************************/
Alignment * compute_prrp_aln (Alignment *A, Constraint_list *CL)
   {
       char *tmpseq=NULL;
       char *tmpaln=NULL;
       char command[10000];
       Sequence *S;

       tmpseq=vtmpnam(NULL);
       tmpaln=vtmpnam(NULL);


       A=seq2aln ( CL->S, A, 1);
       output_gotoh_seq (tmpseq, A);
       sprintf ( command, "prrp -E/dev/null -o%s -F9 %s >/dev/null", tmpaln, tmpseq);
       my_system (command);
       if (!check_file_exists(tmpaln)){return NULL;}
       S=get_fasta_sequence (tmpaln, NULL);

       S->contains_gap=0;
       A=seq2aln(S, A,0);
       free_sequence (S, S->nseq);

       return A;
   }

Alignment *seq2clustalw_aln  (Sequence *S)
{
  return aln2clustalw_aln (seq2aln (S, NULL,RM_GAP), NULL);
}

Alignment * aln2clustalw_aln (Alignment *B, Constraint_list *CL)
{
  char *seq=NULL,*aln=NULL, command[1000];

  output_fasta_seq (seq=vtmpnam (NULL), B);
  sprintf ( command, "clustalw -infile=%s -outorder=input -outfile=%s %s", seq, aln=vtmpnam (NULL), TO_NULL_DEVICE);
  my_system (command);

  if (!check_file_exists(aln))
    return NULL;
  else{B->nseq=0;return main_read_aln(aln,B);}
}
Alignment * compute_tcoffee_aln_quick (Alignment *A, Constraint_list *CL)
   {
       char *tmpseq=NULL;
       char *tmpaln=NULL;
       char command[10000];


       tmpseq=vtmpnam(NULL);
       tmpaln=vtmpnam(NULL);

       if ( CL)A=seq2aln ( CL->S, A, 1);
       output_fasta_seq (tmpseq, A);

       sprintf ( command, "t_coffee  -seq %s -very_fast -outfile %s -quiet ",tmpseq,tmpaln);

       my_system (command);
       if (!check_file_exists(tmpaln))return NULL;
       A->nseq=0;
       A=main_read_aln(tmpaln,A);

       vremove( tmpseq);
       vremove (tmpaln);
       return A;
   }

Alignment * compute_clustalw_aln (Alignment *A, Constraint_list *CL)
   {
       char *tmpseq=NULL;
       char *tmpaln=NULL;
       char command[10000];


       tmpseq=vtmpnam(NULL);
       tmpaln=vtmpnam(NULL);

       A=seq2aln ( CL->S, A, 1);
       output_fasta_seq (tmpseq, A);

       sprintf ( command, "clustalw %sinfile=%s %soutfile=%s %s",CWF, tmpseq,CWF, tmpaln,TO_NULL_DEVICE);

       my_system (command);
       if (!check_file_exists(tmpaln))return NULL;
       A->nseq=0;
       A=main_read_aln(tmpaln,A);

       vremove( tmpseq);
       vremove (tmpaln);
       return A;
   }

Alignment * realign_block ( Alignment *A, int col1, int col2, char *pg)
{
  /*Uses pg: (pg -infile=<infile> -outfile=<outfile> to realign the block [col1 col2[
    Only guaranteed if pg can handle empty sequences
    set pg to NULL to use the default program
  */


  Alignment *L, *M, *R;
  char *seq_name;
  char *aln_name;
  char command[1000], script[1000];



  seq_name=vtmpnam(NULL);
  aln_name=vtmpnam (NULL);

  L=copy_aln (A, NULL);
  M=copy_aln (A, NULL);
  R=copy_aln (A, NULL);

  L=extract_aln ( L, 0, col1);
  M=extract_aln ( M, col1, col2);
  R=extract_aln ( R, col2, A->len_aln);
  output_fasta_seq (seq_name, M);

  sprintf ( script, "%s", (pg==NULL)?"t_coffee":pg);

  sprintf ( command, "%s -infile=%s -outfile=%s %s", script,seq_name, aln_name, TO_NULL_DEVICE);
  my_system ( command);
  free_aln (M);
  M=main_read_aln (aln_name, NULL);


  M=reorder_aln (M, L->name,L->nseq);
  L=aln_cat (L, M);
  L=aln_cat (L, R);
  A=copy_aln (L, A);
  free_aln (L);free_aln (M); free_aln (R);

  return A;
}





/******************************************************************/
/*                  DNA                                           */
/*                                                                */
/*                                                                */
/******************************************************************/


/******************************************************************/
/*                   STRUCTURES                                   */
/*                                                                */
/*                                                                */
/******************************************************************/





Constraint_list * align_pdb_pair_2 (char *seq, Constraint_list *CL)
        {
	    char *tmp_name=NULL;
	    int s1, s2;


	    static char *command;
	    static char *program;

	    tmp_name=vtmpnam ( NULL);

	    if ( !program)program=vcalloc ( LONG_STRING, sizeof (char));
	    if ( !command)command=vcalloc ( LONG_STRING, sizeof (char));

#ifndef     ALIGN_PDB_4_TCOFFEE
	    if ( getenv ( "ALIGN_PDB_4_TCOFFEE")==NULL)crash ("ALIGN_PDB_4_TCOFFEE IS NOT DEFINED");
	    else  sprintf ( program, "%s", (getenv ( "ALIGN_PDB_4_TCOFFEE")));
#else
	    if ( getenv ( "ALIGN_4_TCOFFEE")==NULL)sprintf (program, "%s", ALIGN_PDB_4_TCOFFEE);
	    else sprintf ( program, "%s", (getenv ( "ALIGN_PDB_4_TCOFFEE")));
#endif

	    atoi(strtok (seq,SEPARATORS));
	    s1=atoi(strtok (NULL,SEPARATORS));
	    s2=atoi(strtok (NULL,SEPARATORS));

	    sprintf ( command , "%s -in P%s P%s -gapopen=-40 -max_delta=2.5 -gapext=0 -scale=0 -hasch_mode=hasch_ca_trace_bubble -maximum_distance=10 -output pdb_constraint_list -outfile stdout> %s%s",program, (CL->S)->file[s1], (CL->S)->file[s2], get_cache_dir(),tmp_name);

	    my_system  ( command);
	    CL=read_constraint_list_file(CL, tmp_name);


	    vremove ( tmp_name);


	    return CL;
	}

Constraint_list *align_pdb_pair   (char *seq_in, char *dp_mode,char *evaluate_mode, char *file, Constraint_list *CL, Job_TC *job)
 {
	    int s1, s2;
	    char seq[1000];
	    char name1[1000];
	    char name2[1000];


	    Constraint_list *PWCL;
	    Alignment *F;

	    sprintf ( seq, "%s",seq_in);
	    atoi(strtok (seq,SEPARATORS));
	    s1=atoi(strtok (NULL,SEPARATORS));
	    s2=atoi(strtok (NULL,SEPARATORS));



	    sprintf (name1, "%s%s_%s.%s.align_pdb", get_cache_dir(),(CL->S)->name[s1], (CL->S)->name[s2], dp_mode);
	    sprintf (name2, "%s%s_%s.%s.align_pdb", get_cache_dir(),(CL->S)->name[s2], (CL->S)->name[s1], dp_mode);


	    if ( check_file_exists (name1) &&   is_lib(name1))CL=read_constraint_list_file(CL,name1);
	    else if ( check_file_exists (name2) &&   is_lib(name2))CL=read_constraint_list_file(CL,name2);
	    else
	      {
		PWCL=set_constraint_list4align_pdb ( CL,s1,dp_mode, evaluate_mode, NULL);
		PWCL=set_constraint_list4align_pdb ( CL,s2,dp_mode, evaluate_mode, NULL);
		((job->param)->TCM)->PW_CL=PWCL;
		F=fast_pair (job);
		output_constraints (name1, "100", F);
		CL=aln2constraint_list (F, CL, "100");
		free_aln (F);
	      }
	    return CL;
	}

Constraint_list * profile_pair (TC_method *M , char *in_seq, Constraint_list *CL)
        {

	  char seq[1000];
	  int a, s1, s2;
	  char *result,*prf1_file, *prf2_file;
	  Alignment *F=NULL, *A1, *A2;
	  FILE *fp;
	  char command[10000];
	  char *param;

	  if ( M->executable2[0]=='\0')
	    fprintf ( stderr, "\nERROR: profile_pair requires a method: thread_pair@EP@executable2@<method> [FATAL:%s]\n", PROGRAM);


	  sprintf ( seq, "%s", in_seq);
	  atoi(strtok (seq,SEPARATORS));
	  s1=atoi(strtok (NULL,SEPARATORS));
	  s2=atoi(strtok (NULL,SEPARATORS));

	  A1=seq2R_template_profile(CL->S,s1);
	  A2=seq2R_template_profile(CL->S,s2);


	  prf1_file=vtmpnam (NULL);
	  fp=vfopen (prf1_file, "w");
	  if ( A1)
	    {
	      fprintf (fp, ">%s\n%s\n",(CL->S)->name[s1], aln2cons_seq_mat(A1, "blosum62mt"));
	      for ( a=0; a< A1->nseq; a++)fprintf (fp, ">prf_seq1_%d\n%s\n", a, A1->seq_al[a]);
	    }
	  else
	    {
	      fprintf ( fp, ">%s\n%s\n", (CL->S)->name[s1], (CL->S)->seq[s1]);
	    }
	  vfclose (fp);

	  prf2_file=vtmpnam (NULL);
	  fp=vfopen (prf2_file, "w");
	  if (A2)
	    {
	      fprintf (fp, ">%s\n%s\n",(CL->S)->name[s2], aln2cons_seq_mat(A2, "blosum62mt"));
	      for ( a=0; a< A2->nseq; a++)fprintf (fp, ">prf_seq2_%d\n%s\n", a, A2->seq_al[a]);
	    }
	  else
	    {
	      fprintf ( fp, ">%s\n%s\n", (CL->S)->name[s2], (CL->S)->seq[s2]);
	    }
	  vfclose (fp);

	  result=vtmpnam (NULL);
	  if ( M->param)
	    {
	      param=vcalloc(strlen (M->param)+1, sizeof (char));
	      sprintf ( param, "%s", M->param);
	      param=substitute ( param, " ", "");
	      param=substitute ( param, "\n", "");
	    }

	  sprintf ( command, "tc_generic_method.pl -mode=profile_pair -method=%s %s%s %s%s %s%s -param=%s -tmpdir=%s", M->executable2,M->in_flag,prf1_file, M->in_flag2,prf2_file,M->out_flag, result, param, get_tmp_4_tcoffee());

	  my_system ( command);



	  if ( !check_file_exists (result))
	    {
	      fprintf ( stderr, "\n\tprofile_pair/%s failed:\n\t%s\n",M->executable2, command);
	      myexit (EXIT_FAILURE);
	    }
	  else if ( is_lib (result))
	    {
	      CL=read_constraint_list_file(CL,result);
	    }
	  else if ( is_aln (result))
	    {
	      F=main_read_aln (result, NULL);
	      char *name1, *name2;
	      name1=(CL->S)->name[s1];
	      name2=(CL->S)->name[s2];

	      fp=vfopen (result, "w");
	      for ( a=0; a< F->nseq; a++)
		if (strm ( F->name[a], name1) || strm (F->name[a], name2))
		  fprintf ( fp, ">%s\n%s\n", F->name[a], F->seq_al[a]);
	      vfclose (fp);
	      free_aln (F);
	      F=main_read_aln (result, NULL);
	      CL=aln2constraint_list (F, CL, "sim");
	      free_aln (F);
	    }
	  return CL;
	}
Constraint_list * pdbid_pair (TC_method *M , char *in_seq, Constraint_list *CL)
        {

	  char seq[1000];

	  int s1, s2;
	  char *result, *pdb1, *pdb2;
	  Alignment *F=NULL;
	  char *command;


	  if ( M->executable2[0]=='\0')
	    {
	    fprintf ( stderr, "\nERROR: pdbid_pair requires a structural alignment method: pdb_pair@EP@EXECUTABLE2@<method> [FATAL:%s]\n", PROGRAM);
	    myexit (EXIT_FAILURE);
	    }
	  sprintf ( seq, "%s", in_seq);


	  atoi(strtok (seq,SEPARATORS));
	  s1=atoi(strtok (NULL,SEPARATORS));
	  s2=atoi(strtok (NULL,SEPARATORS));

	  pdb1=seq2P_pdb_id(CL->S,s1);
	  pdb2=seq2P_pdb_id(CL->S,s2);

	  if (!is_pdb_name (pdb1) || !is_pdb_name(pdb2))
	    {
	      return CL;
	    }


	  result=vtmpnam (NULL);
	  command = vcalloc ( 1000, sizeof (char));

	  sprintf ( command, "tc_generic_method.pl -mode=pdbid_pair -method=%s %s%s %s%s %s%s -email=%s -cache=%s -tmpdir=%s", M->executable2,M->in_flag,pdb1, M->in_flag2,pdb2,M->out_flag, result,getenv ("EMAIL"),get_cache_dir(), get_tmp_4_tcoffee());
	  my_system ( command);
	  vfree (command);
	  if (file_is_empty (result))return CL;
	  else
	    {
	      F=main_read_aln (result, NULL);

	      if ( !F)
		{
		  fprintf ( stderr, "\n\tpdb_pair/%s failed:\n\t%s\n",M->executable2, command);
		}
	      else
		{

		  sprintf ( F->name[0],"%s", (CL->S)->name[s1]);
		  sprintf ( F->name[1],"%s", (CL->S)->name[s2]);
		  CL=aln2constraint_list (F, CL, "sim");
		}
	      free_aln (F);
	    }
	  return CL;
	}

Constraint_list * pdb_pair (TC_method *M , char *in_seq, Constraint_list *CL)
        {

	  char seq[1000];
	  int s1, s2;
	  char *result, *pdb1,*pdb1_file, *pdb2, *pdb2_file;
	  Alignment *F=NULL;


	  char command[10000];

	  if ( M->executable2[0]=='\0')
	    {
	    fprintf ( stderr, "\nERROR: pdb_pair requires a structural alignment method: pdb_pair@EP@EXECUTABLE2@<method> [FATAL:%s]\n", PROGRAM);
	    myexit (EXIT_FAILURE);
	    }

	  sprintf ( seq, "%s", in_seq);


	  atoi(strtok (seq,SEPARATORS));
	  s1=atoi(strtok (NULL,SEPARATORS));
	  s2=atoi(strtok (NULL,SEPARATORS));

	  pdb1=seq2P_template_file(CL->S,s1);
	  pdb2=seq2P_template_file(CL->S,s2);
	  if ( !pdb1 || !pdb2) return CL;


	  pdb1_file=vtmpnam (NULL);
	  pdb2_file=vtmpnam (NULL);



	  sprintf ( command, "extract_from_pdb -infile %s -atom ALL -chain FIRST  -nodiagnostic > %s", pdb1, pdb1_file);
	  my_system (command);



	  sprintf ( command, "extract_from_pdb -infile %s -atom ALL -chain FIRST  -nodiagnostic > %s", pdb2, pdb2_file);
	  my_system (command);


	  result=vtmpnam (NULL);

	  sprintf ( command, "tc_generic_method.pl -mode=pdb_pair -method=%s %s%s %s%s %s%s -tmpdir=%s", M->executable2,M->in_flag,pdb1_file, M->in_flag2,pdb2_file,M->out_flag, result, get_tmp_4_tcoffee());
	  my_system ( command);

	  F=main_read_aln (result, NULL);

	  if ( !F)
	    {
	      fprintf ( stderr, "\n\tpdb_pair/%s failed:\n\t%s\n",M->executable2, command);
	    }
	  else
	    {

	      sprintf ( F->name[0],"%s", (CL->S)->name[s1]);
	      sprintf ( F->name[1],"%s", (CL->S)->name[s2]);
	      CL=aln2constraint_list (F, CL, "sim");
	    }


	  free_aln (F);
	  return CL;
	}

Constraint_list * seq_msa (TC_method *M , char *in_seq, Constraint_list *CL)
{
  char seq[1000];
  char *infile, *outfile;
  int a, n, s;
  Alignment *F=NULL;
  FILE *fp;
  char command[1000];


  infile=vtmpnam (NULL);
  outfile=vtmpnam (NULL);

  sprintf ( seq, "%s", in_seq);

  n=atoi(strtok (seq,SEPARATORS));

  fp=vfopen (infile, "w");
  for ( a=0; a<n; a++)
    {
      s=atoi(strtok (NULL,SEPARATORS));

      fprintf (fp, ">%s\n%s\n", (CL->S)->name[s], (CL->S)->seq[s]);
    }
  vfclose (fp);

  sprintf ( command, "t_coffee -other_pg tc_generic_method.pl -mode=seq_msa -method=%s %s%s %s%s -tmpdir=%s", M->executable2, M->in_flag, infile, M->out_flag, outfile, get_tmp_4_tcoffee());
  my_system (command);


  if ( strm (M->out_mode, "aln") ||  strm (M->out_mode, "A"))
    {
      F=main_read_aln (outfile, NULL);
      if ( !F)
	{
	  fprintf ( stderr, "\n\tseq_msa/%s failed:\n\t%s\n", M->executable2,command);
	}
      else
	{
	  CL=aln2constraint_list (F, CL, "sim");
	}
      free_aln (F);
    }
  else if ( strm (M->out_mode, "fL")|| strm (M->out_mode, "lib"))
    {
      Constraint_list *NCL;
      NCL=read_constraint_list_file(CL,outfile);
      if ( !NCL)
	{
	  fprintf ( stderr, "\n\tseq_msa/%s failed:\n\t%s\n", M->executable2,command);
	}
      else
	{
	  CL=NCL;
	}
    }
  return CL;
}
Constraint_list * thread_pair (TC_method *M , char *in_seq, Constraint_list *CL)
        {

	  char seq[1000];
	  int s1, s2;

	  if ( M->executable2[0]=='\0')
	    {
	      fprintf ( stderr, "\nERROR: thread_pair requires a threading method: pdb_pair@EP@EXECUTABLE2@<method> [FATAL:%s]\n", PROGRAM);
	      myexit (EXIT_FAILURE);
	    }

	  sprintf ( seq, "%s", in_seq);
	  atoi(strtok (seq,SEPARATORS));
	  s1=atoi(strtok (NULL,SEPARATORS));
	  s2=atoi(strtok (NULL,SEPARATORS));

	  CL=thread_pair2(M,s1, s2, CL);
	  CL=thread_pair2(M,s2, s1, CL);

	  return CL;
	}



Constraint_list* thread_pair2 ( TC_method *M, int s1, int s2, Constraint_list *CL)
  {
    char *result, *pep, *pdb, *pdb1;
    Alignment *F=NULL;
    Sequence *STL;
    FILE *fp;
    char command[10000];

    STL=(CL)?CL->STRUC_LIST:NULL;


    if ( !(CL->S) || !((CL->S)->T[s1]) || !((CL->S)->T[s1])->P || !seq2P_template_file(CL->S,s1))return CL;
    else pdb1=seq2P_template_file(CL->S,s1);


    pdb=vtmpnam (NULL);
    result=vtmpnam (NULL);
    pep=vtmpnam (NULL);

    sprintf ( command, "extract_from_pdb -infile %s -atom ALL -chain FIRST  -nodiagnostic > %s", pdb1, pdb);
    my_system (command);



    fp=vfopen (pep, "w");
    fprintf ( fp, ">%s\n%s\n",(CL->S)->name[s2],(CL->S)->seq[s2] );
    vfclose (fp);
    sprintf ( command, "tc_generic_method.pl -mode=thread_pair -method=%s %s%s %s%s %s%s -tmpdir=%s", M->executable2,M->in_flag,pep, M->in_flag2,pdb,M->out_flag, result, get_tmp_4_tcoffee());
    my_system ( command);
    F=main_read_aln (result, NULL);

    if ( !F)
      {
	fprintf ( stderr, "\n\tthread_pair/%s failed:\n\t%s\n", M->executable2,command);
      }
    else
      {
    	sprintf ( F->name[0],"%s", (CL->S)->name[s1]);
	sprintf ( F->name[1],"%s", (CL->S)->name[s2]);
	CL=aln2constraint_list (F, CL, "sim");
      }


    free_aln (F);
    return CL;
  }

Constraint_list * lsqman_pair ( char *in_seq, Constraint_list *CL)
        {
	  FILE *fp;
	  static CLIST_TYPE *entry;
	  char command[STRING];
	  char seq[1000];
	  int s1, s2;
	  char *seq_file, *lsqman_result, *tmp_name1;
	  Alignment *F=NULL;
	  int n_failure=0;

	  sprintf ( seq, "%s", in_seq);


	  if ( !entry)entry=vcalloc ( LIST_N_FIELDS, sizeof ( CLIST_TYPE ));

	  atoi(strtok (seq,SEPARATORS));
	  s1=atoi(strtok (NULL,SEPARATORS));
	  s2=atoi(strtok (NULL,SEPARATORS));


	 tmp_name1=vcalloc (100, sizeof (char));
	 sprintf ( tmp_name1, "%s_%s.lsqman_aln", (CL->S)->name[s1], (CL->S)->name[s2]);
	 if ( check_file_exists ( tmp_name1) && (F=main_read_aln(tmp_name1, NULL))!=NULL)
	      {
	        free_aln(F);
		lsqman_result=tmp_name1;
	      }

	 else
	   {
	     seq_file=vtmpnam (NULL);
	     lsqman_result=tmp_name1;
	     fp=vfopen (seq_file, "w");
	     fprintf ( fp, ">%s\n%s\n",(CL->S)->name[s1],(CL->S)->seq[s2] );
	     vfclose (fp);
	     sprintf ( command, "%s -pdb %s -pep %s > %s%s", LSQMAN_4_TCOFFEE, (CL->S)->name[s1], seq_file,get_cache_dir(), lsqman_result);

	     while (!F)
	       {
		 my_system ( command);
		 F=main_read_aln (lsqman_result, NULL);
		 if ( !F)
		   {
		     fprintf ( stderr, "\n\tlsqman failed: will be retried");
		     if ( n_failure==0)fprintf ( stderr, "\n\t%s", command);
		     n_failure++;
		     if ( n_failure==10)
			{
			fprintf ( stderr, "\nCould not run Fugue: will replace it with slow_pair\n");
			vremove (lsqman_result);
			return NULL;
			}
		   }
		 free_aln (F);
	       }
	     vremove( seq_file);

	   }


	 F=main_read_aln(lsqman_result, NULL);
	 /*sprintf ( F->name[0],"%s", (CL->S)->name[s1]);
	 sprintf ( F->name[1],"%s", (CL->S)->name[s2]);
	 */
	 CL=aln2constraint_list (F, CL, "100");
	 free_aln (F);
	 return CL;
	}

Constraint_list * sap_pair   (char *seq, char *weight, Constraint_list *CL)
 {
            register int a;
	    FILE *fp;
	    char full_name[FILENAMELEN];
	    char *tmp_pdb1, *tmp_pdb2;
	    char *sap_seq1, *sap_seq2;
	    char *sap_lib,  *tmp_name, *tmp_name1, *tmp_name2;
	    char *buf=NULL;
	    int s1, s2, r1=0, r2=0;
	    int sim=0, tot=0, score=0;

	    char program[STRING];
	    char *string1, *string2, *string3, *string4, *string5;
	    int max_struc_len=10000;
	    char *template1, *template2;
	    int c;

	    /*check_program_is_installed ( "sap"                   ,SAP_4_TCOFFEE, "SAP_4_TCOFFEE",MAIL, IS_FATAL);*/

	    
	    atoi(strtok (seq,SEPARATORS));
	    s1=atoi(strtok (NULL,SEPARATORS));
	    s2=atoi(strtok (NULL,SEPARATORS));

	    template1=seq2T_value(CL->S,s1, "template_name", "_P_");
	    template2=seq2T_value(CL->S,s2, "template_name", "_P_");

	    
	    if (!template1 || !template2) return CL;


	    declare_name (string1);
	    declare_name (string2);
	    declare_name (string3);
	    declare_name (string4);
	    declare_name (string5);

#ifndef     SAP_4_TCOFFEE
	    if ( getenv ( "SAP_4_TCOFFEE")==NULL)crash ("SAP_4_TCOFFEE IS NOT DEFINED");
	    else  sprintf ( program, "%s", (getenv ( "SAP_4_TCOFFEE")));
#else
	    if ( getenv ( "SAP_4_TCOFFEE")==NULL)sprintf (program, "%s", SAP_4_TCOFFEE);
	    else sprintf ( program, "%s", (getenv ( "SAP_4_TCOFFEE")));
#endif



	    tmp_name1=vcalloc (100, sizeof (char));
	    sprintf ( tmp_name1, "%s_%s.sap_results",template1,template2);
	    tmp_name2=vcalloc (100, sizeof (char));
	    sprintf ( tmp_name2, "%s_%s.sap_results",template2,template1);



	    if (is_sap_file (tmp_name1))
	      {
		tmp_name=tmp_name1;
	      }
	    else if ( is_sap_file (tmp_name2))
	      {
		tmp_name=tmp_name2;
		SWAP (s1, s2);
	      }
	    else
	      {
		tmp_name=tmp_name1;

		tmp_pdb1=normalize_pdb_file(seq2P_template_file(CL->S,s1),(CL->S)->seq[s1], vtmpnam (NULL));
		tmp_pdb2=normalize_pdb_file(seq2P_template_file(CL->S,s2),(CL->S)->seq[s2], vtmpnam (NULL));
		sprintf ( full_name, "%s%s", get_cache_dir (), tmp_name);
		printf_system ("%s %s %s >%s 2>/dev/null",program,tmp_pdb1,tmp_pdb2, full_name);
		if ( !check_file_exists (full_name) || !is_sap_file(full_name))
		  {
		    add_warning ( stderr, "WARNING: SAP failed to align: %s against %s [%s:WARNING]\n", seq2P_template_file(CL->S,s1),seq2P_template_file(CL->S,s2), PROGRAM);
		    if ( check_file_exists (full_name))add2file2remove_list (full_name);
		    return CL;
		  }
		if ( flag_file2remove_is_on())add2file2remove_list (full_name);
		remove ("super.pdb");
	      }



	    sap_seq1=vcalloc (max_struc_len, sizeof (char));
	    sap_seq2=vcalloc (max_struc_len, sizeof (char));

	    fp=find_token_in_file ( tmp_name, NULL, "Percent");
	    fp=find_token_in_file ( tmp_name, fp  , "Percent");
	    while ( (c=fgetc (fp))!='\n' && c!=EOF)fprintf ( fp, "%c", c);
	    while ((buf=vfgets (buf, fp)))
	      {

		if ( !strstr (buf, "eighted") && !strstr (buf, "RMSd"))
		  {
		    remove_charset (buf, "!alnum");
		    r1=buf[0];
		    r2=buf[strlen(buf)-1];
		  }
		else
		  continue;

		sim+=(r1==r2)?1:0;
		if ( tot>max_struc_len)
		  {max_struc_len+=max_struc_len;
		    sap_seq1=vrealloc ( sap_seq1, sizeof(char)*max_struc_len);
		    sap_seq2=vrealloc ( sap_seq2, sizeof(char)*max_struc_len);
		  }
		sap_seq1[tot]=r1;
		sap_seq2[tot]=r2;
		tot++;
	      }
	    sim=(sim*100)/tot;

	    if ( is_number (weight))score=atoi(weight);
	    else if ( strstr ( weight, "OW"))
	      {
		int ow;
		sscanf ( weight, "OW%d", &ow);
		score=sim*ow;
	      }
	    else
	      {
		score=sim;
	      }


	    vfclose (fp);
	    sap_seq1[tot]=sap_seq2[tot]='\0';


	    fp=vfopen ( sap_lib=vtmpnam(NULL), "w");
	    fprintf (fp, "! TC_LIB_FORMAT_01\n");
	    fprintf (fp, "2\n");
	    fprintf (fp, "%s %d %s\n", (CL->S)->name[s2],(int)strlen (sap_seq1), sap_seq1);
	    fprintf (fp, "%s %d %s\n", (CL->S)->name[s1],(int)strlen (sap_seq2), sap_seq2);
	    fprintf (fp, "#1 2\n");



	    //HERE ("\n%s\n%s\n%s\n\n%s\n%s\n%s", (CL->S)->name[s1],(CL->S)->seq[s1], sap_seq1, (CL->S)->name[s2],(CL->S)->seq[s2], sap_seq2);exit (0); 
	    for ( a=0; a< tot; a++)
	      {
		fprintf (fp, "%d %d %d 1 0\n", a+1, a+1, score);
	      }
	    
	    fprintf (fp, "! CPU 0\n");
	    fprintf (fp, "! SEQ_1_TO_N\n");
	    vfclose (fp);
	    
	    CL=read_constraint_list_file(CL,sap_lib);
	    vremove (sap_lib);

	    vfree ( string1);vfree ( string2);vfree ( string3);vfree ( string4);vfree ( string5);
	    vfree (sap_seq1); vfree(sap_seq2);vfree (tmp_name1); vfree(tmp_name2);
	    vfree (buf);
	    return CL;
	}



Constraint_list *rna_pair (TC_method *M ,
						   char *in_seq,
						   Constraint_list *CL)
{
	char seq[1000];
	int s1, s2;
	char *result, *pdb1, *pdb2, *pdb1_file, *pdb2_file;
	Alignment *F=NULL;



	char command[10000];

	if ( M->executable2[0]=='\0')
	{
		fprintf ( stderr, "\nERROR: rna_pair requires a structural alignment method: rna_pair@EP@EXECUTABLE2@<method> [FATAL:%s]\n", PROGRAM);
		myexit (EXIT_FAILURE);
	}

	sprintf ( seq, "%s", in_seq);


	atoi(strtok (seq,SEPARATORS));
	s1=atoi(strtok (NULL,SEPARATORS));
	s2=atoi(strtok (NULL,SEPARATORS));

	pdb1=seq2P_template_file(CL->S,s1);
	pdb1_file=vtmpnam (NULL);
	sprintf ( command, "extract_from_pdb -infile %s -atom ALL -chain FIRST  -nodiagnostic > %s", pdb1, pdb1_file);
	my_system (command);
	//
	pdb2=seq2P_template_file(CL->S,s2);
	pdb2_file=vtmpnam (NULL);
	sprintf ( command, "extract_from_pdb -infile %s -atom ALL -chain FIRST  -nodiagnostic > %s", pdb2, pdb2_file);
	my_system (command);

	result=vtmpnam (NULL);

	sprintf ( command, "tc_generic_method.pl -mode=rna_pair -method=%s %s%s %s%s %s%s -tmpdir=%s", M->executable2,M->in_flag,pdb1_file, M->in_flag2,pdb2_file,M->out_flag, result, get_tmp_4_tcoffee());

	my_system ( command);

	F=main_read_aln (result, NULL);


	if ( !F)
	{
		fprintf ( stderr, "\n\trna_pair/%s failed:\n\t%s\n",M->executable2, command);
	}
	else
	{
		sprintf ( F->name[0],"%s", (CL->S)->name[s1]);
		sprintf ( F->name[1],"%s", (CL->S)->name[s2]);
		CL=aln2constraint_list (F, CL, "sim");
	}


	free_aln (F);
	return CL;
}


/******************************************************************/
/*                   GENERIC PAIRWISE METHODS                     */
/*                                                                */
/*                                                                */
/******************************************************************/



Constraint_list * best_pair4rna(Job_TC *job)
{
	int n,a;


	static char *seq;
	Alignment *A;
	Constraint_list *PW_CL;
	Constraint_list *CL, *RCL;
	char *seq_in;
	Sequence *S;
	TC_method *M, *sara_pairM, *proba_pairM;
	int*seqlist;

	int s1, s2;
	Template *T1, *T2;
	int ml=0;
	struct X_template *r1, *r2, *p1, *p2;
	static int **blosum;

	if (!seq)seq=vcalloc (100, sizeof (char));

	A=(job->io)->A;
	M=(job->param)->TCM;
	CL=(job->io)->CL;
	S=CL->S;
	for (a=0; a<S->nseq; a++)ml=MAX(ml, strlen (S->name[a]));


	if ( !strm ( retrieve_seq_type(), "RNA") && !strm ( retrieve_seq_type(), "DNA"))
		printf_exit (EXIT_FAILURE, stderr, "ERROR: RNA Sequences Only with best4rna_pair  [FATAL:%s]\n",PROGRAM);


	seq_in=(job->param)->seq_c;
	sprintf (seq, "%s", seq_in);
	seqlist=string2num_list (seq);
	n=seqlist[1];
	if ( n!=2){fprintf ( stderr, "\nERROR: best_pair can only handle two seq at a time [FATAL]\n");myexit (EXIT_FAILURE);}
	s1=seqlist[2];
	s2=seqlist[3];

	T1=S->T[s1];
	T2=S->T[s2];
	r1=T1->R;
	r2=T2->R;
	p1=T1->P;
	p2=T2->P;

	PW_CL=((job->param)->TCM)->PW_CL;
	CL=(job->io)->CL;

	if (!blosum)blosum=read_matrice ("blosum62mt");

// 	id=idscore_pairseq (S->seq[s1], S->seq[s2],-10,-1,blosum, "sim");


	proba_pairM=method_file2TC_method(method_name2method_file ("proba_pair"));
	proba_pairM->PW_CL=method2pw_cl(proba_pairM, CL);

	sara_pairM=method_file2TC_method(method_name2method_file ("sara_pair"));
	sara_pairM->PW_CL=method2pw_cl(sara_pairM, CL);

	if ( p1 && p2)
	{
      //Avoid Structural Tem
		T1->R=NULL;
		T2->R=NULL;
		fprintf ( stderr, "\n\t%-*s %-*s: Structure Based Alignment\n", ml,S->name[s1], ml,S->name[s2]);
		(job->param)->TCM=sara_pairM;
	}
	else
	{
		fprintf ( stderr, "\n\t%-*s %-*s: Direct Sequence Alignment\n", ml,S->name[s1], ml,S->name[s2]);
		(job->param)->TCM=proba_pairM;
	}

	RCL=seq2list (job);
	T1->R=r1;
	T2->R=r2;

	return RCL;
}

Constraint_list * best_pair4prot      (Job_TC *job)
{
  int n,a;


  static char *seq;
  Alignment *A;
  Constraint_list *PW_CL;
  Constraint_list *CL, *RCL;
  char *seq_in;
  Sequence *S;
  TC_method *M, *sap_pairM, *proba_pairM;
  int*seqlist;

  int id, s1, s2;
  Template *T1, *T2;
  int ml=0;
  struct X_template *r1, *r2, *p1, *p2;
  static int **blosum;

  if (!seq)seq=vcalloc (100, sizeof (char));

  A=(job->io)->A;
  M=(job->param)->TCM;
  CL=(job->io)->CL;
  S=CL->S;
  for (a=0; a<S->nseq; a++)ml=MAX(ml, strlen (S->name[a]));


  if ( strm ( retrieve_seq_type(), "DNA") ||strm ( retrieve_seq_type(), "RNA") )printf_exit (EXIT_FAILURE, stderr, "ERROR: Protein Sequences Only with bestprot_pair  [FATAL:%s]\n",PROGRAM);


  seq_in=(job->param)->seq_c;
  sprintf (seq, "%s", seq_in);
  seqlist=string2num_list (seq);
  n=seqlist[1];
  if ( n!=2){fprintf ( stderr, "\nERROR: best_pair can only handle two seq at a time [FATAL]\n");myexit (EXIT_FAILURE);}
  s1=seqlist[2];
  s2=seqlist[3];

  T1=S->T[s1];
  T2=S->T[s2];
  r1=T1->R;
  r2=T2->R;
  p1=T1->P;
  p2=T2->P;

  PW_CL=((job->param)->TCM)->PW_CL;
  CL=(job->io)->CL;

  if (!blosum)blosum=read_matrice ("blosum62mt");

  id=idscore_pairseq (S->seq[s1], S->seq[s2],-10,-1,blosum, "sim");


  proba_pairM=method_file2TC_method(method_name2method_file ("proba_pair"));
  proba_pairM->PW_CL=method2pw_cl(proba_pairM, CL);

  sap_pairM=method_file2TC_method(method_name2method_file ("sap_pair"));
  sap_pairM->PW_CL=method2pw_cl(sap_pairM, CL);

  if ( id>80)
    {
      //Hide The Template
      T1->R=NULL;
      T2->R=NULL;
      fprintf ( stderr, "\n\t%-*s %-*s: Direct Sequence Alignment\n", ml,S->name[s1], ml,S->name[s2]);
      (job->param)->TCM=proba_pairM;
    }
  else if ( p1 && p2)
    {
      //Avoid Structural Tem
      T1->R=NULL;
      T2->R=NULL;
      fprintf ( stderr, "\n\t%-*s %-*s: Structure Based Alignment\n", ml,S->name[s1], ml,S->name[s2]);
      (job->param)->TCM=sap_pairM;
    }
  else if ( r1 || r2)
    {
      fprintf ( stderr, "\n\tt%-*s %-*s: PSIBLAST Profile Alignment\n", ml,S->name[s1], ml,S->name[s2]);
      (job->param)->TCM=proba_pairM;
    }
  else
    {
       fprintf ( stderr, "\n\t%-*s %-*s: Direct Sequence Alignment (No Profile)\n", ml,S->name[s1], ml,S->name[s2]);
       (job->param)->TCM=proba_pairM;
    }

  RCL=seq2list (job);
  T1->R=r1;
  T2->R=r2;

  return RCL;
}


Alignment * fast_pair      (Job_TC *job)
        {
	    int s, n,a;
	    static int **l_s;
	    static int *ns;
	    char seq[1000];
	    Alignment *A;
	    Constraint_list *PW_CL;
	    Constraint_list *CL;
	    char *seq_in;
	    Sequence *S;
	    TC_method *M;
	    int*seqlist;
	    char **buf;

	    A=(job->io)->A;

	    M=(job->param)->TCM;
	    PW_CL=((job->param)->TCM)->PW_CL;
	    CL=(job->io)->CL;
	    seq_in=(job->param)->seq_c;


	    sprintf (seq, "%s", seq_in);
	    seqlist=string2num_list (seq);
	    n=seqlist[1];
	    if ( n!=2){fprintf ( stderr, "\nERROR: fast_pw_aln can only handle two seq at a time [FATAL]\n");myexit (EXIT_FAILURE);}

	    S=(CL)->S;

	    if (!A) {A=declare_aln (CL->S);}
	    if ( !ns)
	        {
		ns=vcalloc ( 2, sizeof (int));
		l_s=declare_int (2,(CL->S)->nseq);
		}
	    buf=vcalloc ( S->nseq, sizeof (char*));

	    for ( a=0; a< n; a++)
	        {
		  s=seqlist[a+2];
		  if ( strm (M->seq_type, "G"))
		    {
		      buf[s]=S->seq[s];
		      S->seq[s]=((((S->T[s])->G)->VG)->S)->seq[0];
		  }
		else
		  buf[s]=S->seq[s];


		  sprintf ( A->seq_al[a], "%s",S->seq[s]);
		  sprintf ( A->name[a], "%s", (CL->S)->name[s]);
		  A->order[a][0]=s;
		}

	    A->S=CL->S;
	    PW_CL->S=CL->S;
	    A->CL=CL;
	    ns[0]=ns[1]=1;
	    l_s[0][0]=0;
	    l_s[1][0]=1;


	    if (PW_CL->reverse_seq)
	      {
		invert_string2(A->seq_al[0]);
		invert_string2(A->seq_al[1]);
		invert_string2 ((CL->S)->seq[A->order[0][0]]);
		invert_string2 ((CL->S)->seq[A->order[1][0]]);

	      }

	    pair_wise ( A, ns, l_s, PW_CL);

	    if (PW_CL->reverse_seq)
	      {

		invert_string2(A->seq_al[0]);
		invert_string2(A->seq_al[1]);
		invert_string2 ((CL->S)->seq[A->order[0][0]]);
		invert_string2 ((CL->S)->seq[A->order[1][0]]);
	      }
	    A->nseq=n;

	    for ( a=0; a<S->nseq; a++)
	      {
		if ( !buf[a] || buf[a]==S->seq[a]);
		else S->seq[a]=buf[a];
	      }
	    vfree (buf);vfree (seqlist);
	    return A;

	}
Alignment * align_two_aln ( Alignment *A1, Alignment  *A2, char *in_matrix, int gop, int gep, char *in_align_mode)
        {
	Alignment *A=NULL;
	Constraint_list *CL;
	Sequence *S;
	int a;
	int *ns;
	int **ls;
	static char *matrix;
	static char *align_mode;

	if (!matrix)matrix=vcalloc ( 100, sizeof (char));
	if (!align_mode)align_mode=vcalloc ( 100, sizeof (char));
	


	sprintf ( matrix, "%s", in_matrix);
	sprintf ( align_mode, "%s", in_align_mode);

	CL=vcalloc ( 1, sizeof (Constraint_list));
	CL->pw_parameters_set=1;
	CL->M=read_matrice (matrix);
	CL->matrices_list=declare_char (10, 10);

	CL->evaluate_residue_pair=evaluate_matrix_score;
	CL->get_dp_cost=consensus_get_dp_cost;
	CL->normalise=1;

	CL->extend_jit=0;
	CL->maximise=1;
	CL->gop=gop;
	CL->gep=gep;
	CL->TG_MODE=2;
	sprintf (CL->matrix_for_aa_group, "vasiliky");
	CL->use_fragments=0;
	CL->ktup=5;
	if ( !CL->use_fragments)CL->diagonal_threshold=0;
	else CL->diagonal_threshold=6;

	sprintf (CL->dp_mode, "%s", align_mode);

	A=copy_aln (A1, A);
	A=stack_aln (A, A2);
	CL->S=fill_sequence_struc(A->nseq, A->seq_al,A->name);

	ns=vcalloc ( 2, sizeof(int));
	ls=declare_int ( 2,A->nseq);
	ns[0]=A1->nseq;
	ns[1]=A2->nseq;
	for ( a=0; a<ns[0]; a++)
	  ls[0][a]=a;
	for ( a=0; a< ns[1]; a++)
	  ls[1][a]=a+A1->nseq;

	A->score_aln=pair_wise (A, ns, ls,CL);

	vfree (ns);
	free_int (ls, -1);
	S=free_constraint_list (CL);
	free_sequence (S,-1);
	A->S=NULL;
	return A;
	}

static int align_two_seq_keep_case;
void toggle_case_in_align_two_sequences(int value)
{
  align_two_seq_keep_case=value;
}
Alignment * align_two_sequences ( char *seq1, char *seq2, char *in_matrix, int gop, int gep, char *in_align_mode)
        {
	static Alignment *A;
	Constraint_list *CL;
	Sequence *S;

	int *ns;
	int **l_s;

	char       **seq_array;
	char       **name_array;
	static char *matrix;
	static int  **M;

	static char *align_mode;

	if (!matrix)matrix=vcalloc ( 100, sizeof (char));
	if (!align_mode)align_mode=vcalloc ( 100, sizeof (char));
	sprintf ( align_mode, "%s", in_align_mode);

	CL=vcalloc ( 1, sizeof (Constraint_list));

	CL->pw_parameters_set=1;

	CL->matrices_list=declare_char (10, 10);


	if ( !strm (matrix, in_matrix))
	  {
	    sprintf ( matrix,"%s", in_matrix);
	    M=CL->M=read_matrice (matrix);

	  }
	else
	  {
	    CL->M=M;
	  }

	if (strstr (in_align_mode, "cdna"))
	  CL->evaluate_residue_pair=evaluate_cdna_matrix_score;
	else
	  CL->evaluate_residue_pair=evaluate_matrix_score;

	CL->get_dp_cost=get_dp_cost;
	CL->extend_jit=0;
	CL->maximise=1;
	CL->gop=gop;
	CL->gep=gep;
	CL->TG_MODE=2;
	sprintf (CL->matrix_for_aa_group, "vasiliky");
	CL->use_fragments=0;
	CL->ktup=3;
	if ( !CL->use_fragments)CL->diagonal_threshold=0;
	else CL->diagonal_threshold=6;

	sprintf (CL->dp_mode, "%s", align_mode);

	seq_array=declare_char ( 2, MAX(strlen(seq1), strlen (seq2))+1);
	sprintf (seq_array[0], "%s",seq1);
	sprintf (seq_array[1],"%s", seq2);
	ungap_array(seq_array,2);
	if (align_two_seq_keep_case !=KEEP_CASE)string_array_lower(seq_array,2);

	name_array=declare_char (2, STRING);
	sprintf ( name_array[0], "A");
	sprintf ( name_array[1], "B");


	ns=vcalloc ( 2, sizeof(int));
	l_s=declare_int ( 2, 1);
	ns[0]=ns[1]=1;
	l_s[0][0]=0;
	l_s[1][0]=1;



	CL->S=fill_sequence_struc(2, seq_array, name_array);

	A=seq2aln(CL->S, NULL, 1);

	ungap (A->seq_al[0]);
	ungap (A->seq_al[1]);



	A->score_aln=pair_wise (A, ns, l_s,CL);

	vfree (ns);
	free_int (l_s, -1);
	free_char (name_array, -1);free_char ( seq_array,-1);

	CL->M=NULL;
        S=free_constraint_list (CL);
	free_sequence (S,-1);
	A->S=NULL;
	return A;
	}


NT_node make_root_tree ( Alignment *A,Constraint_list *CL,int gop, int gep,Sequence *S,  char *tree_file,int maximise)
{
   NT_node **T=NULL;
   T=make_tree (A, CL, gop, gep,S,tree_file,maximise);
   (T[3][0])->nseq=S->nseq;
   return T[3][0];
}
NT_node ** make_tree ( Alignment *A,Constraint_list *CL,int gop, int gep,Sequence *S,  char *tree_file,int maximise)
	{
	  int a, b, ra, rb;
	NT_node **T=NULL;
	int **distances;
	int out_nseq;
	char **out_seq_name;
	char **out_seq;


	if ( !CL || !CL->tree_mode || !CL->tree_mode[0])
	  {
	    fprintf ( stderr, "\nERROR: No CL->tree_mode specified (make_tree::util_dp_drivers.c [FATAL:%s]", PROGRAM);
	    myexit (EXIT_FAILURE);
	  }
	else
	  fprintf (CL->local_stderr , "\nMAKE GUIDE TREE \n\t[MODE=%s][",CL->tree_mode);


	if ( A->nseq==2)
	  {
	    int tot_node;
	    char *tmp;
	    FILE *fp;
	    fprintf (CL->local_stderr, "---Two Sequences Only: Make Dummy Pair-Tree ---]");
	    tmp=vtmpnam (NULL);
	    fp=vfopen (tmp,"w");
	    fprintf ( fp, "(%s:0.1, %s:0.1):0.1;\n",S->name[0], S->name[1]);
	    vfclose (fp);
	    T=read_tree (tmp, &tot_node, (CL->S)->nseq,(CL->S)->name);

	    return T;
	  }
	else if ( strm (CL->tree_mode, "cwph"))
	  {
	    return  seq2cw_tree ( S, tree_file);
	  }
	else if (strm ( CL->tree_mode, "upgma") || strm ( CL->tree_mode, "nj"))
	  {
	    out_nseq=S->nseq;
	    out_seq_name=S->name;
	    out_seq=S->seq;

	    CL->DM=cl2distance_matrix (CL, NOALN,NULL,NULL,0);

	    if ( CL->S!=S)
	      {
		/*Shrink the distance matrix so that it only contains the required sequences*/
		distances=declare_int (S->nseq, S->nseq);
		for (a=0; a< S->nseq; a++)
		  {
		    ra=name_is_in_list ((S)->name[a],(CL->S)->name, (CL->S)->nseq, 100);
		    for ( b=0; b< S->nseq; b++)
		      {
			rb=name_is_in_list ((S)->name[b],(CL->S)->name, (CL->S)->nseq, 100);
			distances[a][b]=(CL->DM)->score_similarity_matrix[ra][rb];
		      }
		  }
	      }
	    else
	      {
		distances=duplicate_int ( (CL->DM)->score_similarity_matrix, -1, -1);
	      }



	    distances=sim_array2dist_array (distances, MAXID*SCORE_K);
	    distances=normalize_array (distances, MAXID*SCORE_K, 100);
	    if ( strm (CL->tree_mode, "order"))
	      {
		for ( a=0; a< S->nseq; a++)
		  for ( b=0; b< S->nseq; b++)
		    distances[b][a]=100;
		T=make_nj_tree (A,distances,gop,gep,out_seq,out_seq_name,out_nseq, tree_file, CL->tree_mode);
	      }
	    else if ( strm (CL->tree_mode, "nj"))
	      {
		T=make_nj_tree (A,distances,gop,gep,out_seq,out_seq_name,out_nseq, tree_file, CL->tree_mode);
	      }
	    else if ( strm (CL->tree_mode, "upgma"))
	      T=make_upgma_tree (A,distances,gop,gep,out_seq,out_seq_name,out_nseq, tree_file, CL->tree_mode);
	    else
	      {
		printf_exit (EXIT_FAILURE, stderr, "ERROR: %s is an unknown tree computation mode [FATAL:%s]", CL->tree_mode, PROGRAM);
	      }
	    free_int (distances, out_nseq);

	  }

	fprintf (CL->local_stderr , "DONE]\n");
	return T;
	}


Alignment *recompute_local_aln (Alignment *A, Sequence *S,Constraint_list *CL, int scale, int gep)
    {
    int **coor;
    int a;
    Alignment *B;

    sort_constraint_list (CL, 0, CL->ne);
    coor=declare_int (A->nseq, 3);
    for ( a=0; a< A->nseq; a++)
        {
        coor[a][0]=A->order[a][0];
	coor[a][1]=A->order[a][1]+1;
	coor[a][2]=strlen(S->seq[A->order[a][0]])-coor[a][1];
	}
    B=stack_progressive_nol_aln_with_seq_coor(CL,0,0,S,coor,A->nseq);
    A=copy_aln ( B, A);

    CL=compact_list (CL,"best");
    free_Alignment(B);
    return A;
    }


Alignment *stack_progressive_nol_aln_with_seq_coor(Constraint_list *CL,int gop, int gep,Sequence *S, int **seq_coor, int nseq)
    {

    static int ** local_coor1;
    static int ** local_coor2;
    if ( local_coor1!=NULL)free_int (local_coor1, -1);
    if ( local_coor2!=NULL)free_int (local_coor2, -1);

    local_coor1=get_nol_seq          ( CL,seq_coor, nseq, S);
    local_coor2=minimise_repeat_coor ( local_coor1, nseq, S);

    return stack_progressive_aln_with_seq_coor(CL,gop, gep,S, local_coor2,nseq);
    }


Alignment *stack_progressive_aln_with_seq_coor (Constraint_list*CL,int gop, int gep, Sequence *S, int **coor, int nseq)
    {
    Alignment *A=NULL;

    A=seq_coor2aln (S,NULL, coor, nseq);

    return stack_progressive_aln ( A,CL, gop, gep);
    }

Alignment *est_progressive_aln(Alignment *A, Constraint_list *CL, int gop, int gep)
    {
    int a,n;
    int**group_list;
    int *n_groups;
    char *seq;
    n_groups=vcalloc ( 2, sizeof (int));
    group_list=declare_int ( 2, A->nseq);

    n=A->nseq;

    n_groups[0]=1;
    n_groups[1]=1;
    group_list[0][0]=0;
    group_list[0][1]=1;

    group_list[1][0]=1;
    fprintf ( stderr, "\n");
    for ( a=1; a<n; a++)
        {
	sprintf ( A->seq_al[1], "%s", A->seq_al[a]);
	fprintf ( stderr, "\t[%30s]->[len=%5d]", A->name[a],(int)strlen ( A->seq_al[0]));
	pair_wise ( A,n_groups, group_list, CL);

	seq=dna_aln2cons_seq(A);

	sprintf ( A->seq_al[0], "%s", seq);
	vfree (seq);
	fprintf ( stderr, "\n");
	}

    A->nseq=1;
    return A;
    }

void analyse_seq ( Alignment *A, int s)
   {
     int a, b, c;
     int r;

     int len=0;


     int state=0;
     int pstate=-1;
     float score=0;

     for ( a=0; a< A->len_aln; a++)
       {
	 for ( b=0, c=0; b< s; b++)
	   if ( !is_gap(A->seq_al[b][a])){c=1; break;}

	 r=!is_gap(A->seq_al[s][a]);

	 if      (  r &&  c) state=1;
	 else if ( !r && !c) state=2;
	 else if ( !r &&  c) state=3;
	 else if (  r && !c) state=4;

	 if ( state !=pstate)
	   {
	     score+=len*len;
	     len=0;
	   }
	 len+=r;
	 pstate=state;
       }
     score=score/(float)(((A->S)->len[s]*(A->S)->len[s]));
     fprintf ( stderr, "[%.2f]", score);

     return;
   }

Alignment *realign_aln ( Alignment*A, Constraint_list *CL)
{
  int a, b, c;
  int *ns, **ls;
  A=reorder_aln (A, (CL->S)->name,(CL->S)->nseq);

  ns=vcalloc (2, sizeof(int));
  ls=declare_int ( 2, A->nseq);

  for (a=0; a< A->nseq; a++)
    {
      ns[0]=A->nseq-1;
      for (c=0,b=0; b<A->nseq; b++)if (b!=a)ls[0][c++]=b;
      ungap_sub_aln ( A, ns[0], ls[0]);

      ns[1]=1;
      ls[1][0]=a;
      ungap_sub_aln ( A, ns[1], ls[1]);
      A->score_aln=pair_wise (A, ns, ls,CL);
    }

  vfree (ns); free_int (ls, -1);
  return A;
}

Alignment *realign_aln_random_bipart ( Alignment*A, Constraint_list *CL)
{
  int *ns;
  int **l_s;

  int  a,g;

  ns=vcalloc (2, sizeof (int));
  l_s=declare_int (2,A->nseq);

  for ( a=0; a< A->nseq; a++)
    {
    g=rand()%2;
    l_s[g][ns[g]++]=a;
    }

  fprintf ( stderr, "\n");
  ungap_sub_aln ( A, ns[0], l_s[0]);
  ungap_sub_aln ( A, ns[1], l_s[1]);

  /* //Display Groups
  for (a=0;a<2; a++)
    for (b=0; b<ns[a]; b++)
      fprintf ( stderr, "[%d-%d]", a, l_s[a][b]);
  */
  A->score_aln=pair_wise (A, ns, l_s,CL);

  vfree(ns);free_int(l_s, -1);
  return A;
}
Alignment *realign_aln_random_bipart_n ( Alignment*A, Constraint_list *CL, int n)
{
  int *ns;
  int **ls;
  int *used;

  int  a,b,c, p;

  if (n>=A->nseq)n=A->nseq/2;
  used=vcalloc (A->nseq, sizeof (int));
  c=0;
  while (c<n)
    {
      p=rand()%A->nseq;
      if (!used[p]){used[p]=1;c++;}
    }
  ns=vcalloc (2, sizeof (int));
  ls=declare_int (2,A->nseq);
  for (a=0; a<2; a++)
    {
      for (b=0; b<A->nseq; b++)
	if (used[b]==a)ls[a][ns[a]++]=b;
    }
  ungap_sub_aln ( A, ns[0], ls[0]);
  ungap_sub_aln ( A, ns[1], ls[1]);


  A->score_aln=pair_wise (A, ns, ls,CL);
  vfree(ns);free_int(ls, -1);vfree (used);
  return A;
}
int ** seq2ecl_mat (Constraint_list *CL);
int ** seq2ecl_mat (Constraint_list *CL)

{
  int a, b, n;

  Alignment *A;
  int *ns, **ls;
  int **dm;

  ns=vcalloc (2, sizeof (int));
  ls=declare_int ((CL->S)->nseq, 2);

  A=seq2aln (CL->S,NULL, RM_GAP);
  n=(CL->S)->nseq;
  dm=declare_int (n, n);
  for (a=0; a<(CL->S)->nseq-1; a++)
    for (b=a+1; b<(CL->S)->nseq; b++)
      {
	ns[0]=ns[1]=1;
	ls[0][0]=a;
	ls[1][0]=b;
	ungap (A->seq_al[a]);
	ungap (A->seq_al[b]);
	dm[a][b]=dm[b][a]=linked_pair_wise (A, ns, ls, CL);
      }

  return dm;
}
Alignment *realign_aln_clust ( Alignment*A, Constraint_list *CL)
{
  int *ns;
  int **ls;

  int a, b, c,n;
  static int **rm, **dm, **target;
  int score;



  if (!A)
    {
      free_int (dm, -1); free_int (rm, -1);free_int (target, -1);
      dm=rm=target=NULL;
    }


  if (!rm)rm=seq2ecl_mat(CL);
  if (!dm)dm=declare_int (A->nseq, A->nseq);
  if (!target)target=declare_int (A->nseq*A->nseq, 3);

  ns=vcalloc (2, sizeof (int));
  ls=declare_int (2,A->nseq);


  for (a=0; a<A->nseq-1; a++)
    for (b=a+1; b<A->nseq; b++)
      {
	ns[0]=2;
	ls[0][0]=a;
	ls[0][1]=b;
	score=sub_aln2ecl_raw_score (A, CL, ns[0], ls[0]);
	dm[a][b]=dm[b][a]=MAX(0,(rm[a][b]-score));
      }
  for (n=0,a=0; a<A->nseq; a++)
    {
      for (b=a; b<A->nseq; b++, n++)
	{

	  target[n][0]=a;
	  target[n][1]=b;
	  for ( c=0; c<A->nseq; c++)
	    {
	      if (c!=a && c!=b)target[n][2]+=dm[a][c]+dm[b][c];
	    }
	}
    }
  sort_int_inv (target,3, 2, 0, n-1);

  for (a=0; a<A->nseq; a++)
    {
      if (target[a][0]==target[a][1])
	{
	  ns[0]=1;
	  ls[0][0]=target[a][0];
	}
      else
	{
	  ns[0]=2;
	  ls[0][0]=target[a][0]; ls[0][1]=target[a][1];
	}

      for (ns[1]=0,b=0; b<A->nseq; b++)
	{
	  if (b!=target[a][0] && b!=target[a][1])ls[1][ns[1]++]=b;
	}

      ungap_sub_aln (A, ns[0], ls[0]);
      ungap_sub_aln (A, ns[1], ls[1]);

      A->score_aln=pair_wise (A, ns, ls,CL);
      fprintf ( stderr, "\nSEQ: %d %d SCORE=%d\n",target[a][0],target[a][1], aln2ecl_raw_score(A, CL));
    }
  return A;
}

int get_best_group ( int **used, Constraint_list *CL);
int seq_aln_thr1(Alignment *A, int **used, int threshold, Constraint_list *CL);
int seq_aln_thr2( Alignment*A, int **used, int threshold, int g, Constraint_list *CL);

int get_best_group ( int **used, Constraint_list *CL)
{
  int a,b,c,d,n, tot,stot, best_tot, best_seq, nseq;
  int ns[2];
  int *ls[2];

  best_seq=0;
  nseq=((CL->S)->nseq);
  tot=best_tot=0;
  for (a=0; a<nseq; a++)
    {
      if ( used[a][0]==-1)continue;
      for ( tot=0,b=0; b< nseq; b++)
	{
	  if ( a==b) continue;
	  if ( used[b][0]==-1)continue;
	  ns[0]=used[a][1];
	  ls[0]=used[a]+2;
	  ns[1]=used[b][1];
	  ls[1]=used[b]+2;
	  for (stot=0, n=0,c=0; c<ns[0]; c++)
	    for (d=0; d<ns[1]; d++, n++)
	      {
		stot+=(CL->DM)->similarity_matrix[ls[0][c]][ls[1][d]];
	      }
	  if (n>0)stot/=n;
	  tot+=stot;
	}
      if (tot>best_tot)
	{
	  best_tot=tot;
	  best_seq=a;
	}
    }
  return best_seq;
}



Alignment * seq2aln_group (Alignment *A, int N, Constraint_list *CL)
{

  NT_node P;
  int a;

  char *list, **list2;
  Alignment *F;


  fprintf (CL->local_stderr, "\n##### DPA ##### Compute Fast Alignment");
  A=iterative_tree_aln (A,1, CL);
  fprintf (CL->local_stderr, "\n##### DPA ##### Identify Nodes");
  P=make_root_tree (A, CL, CL->gop, CL->gep,CL->S,NULL, 1);
  set_node_score (A, P, "idmat_sim");
  fprintf (CL->local_stderr, "\n##### DPA ##### Split Nodes");
  list=split_nodes_nseq (A,P,N, list=vcalloc (P->nseq*200, sizeof (char)));

  list2=string2list (list);
  fprintf (CL->local_stderr, "\n##### DPA ##### Save Nodes");

  F=A;
  for (a=1; a<atoi (list2[0]); a++)
    {
      A->A=main_read_aln(list2[a], NULL);
      A=A->A;
    }
  fprintf (CL->local_stderr, "\n##### DPA ##### Finished");
  vfree (list); free_char (list2, -1);

  A=F;
  while (A)
    {
            A=A->A;
    }

  return F;
}




Alignment * seq_aln ( Alignment*A, int n,Constraint_list *CL)
{

  int **used,  a, t,n1, nseq;


  n1=nseq=(CL->S)->nseq;
  used=declare_int (nseq, nseq+3);


  for (a=0; a< nseq; a++)
    {
      used[a][1]=1;
      used[a][2]=a;
    }


  for (t=50; t>=0 && nseq>1; t-=5)
    {
      nseq=seq_aln_thr1 (A, used,t, CL);
    }

  vfree (used);
  return A;
}

int seq_aln_thrX(Alignment *A, int **used, int threshold, Constraint_list *CL)
{
  int n=0,a;
  seq_aln_thr1(A,used,threshold,CL);
  for ( a=0; a< (CL->S)->nseq; a++)
    n+=(used[a][1]>0)?1:0;

  return n;
}
int seq_aln_thr1(Alignment *A, int **used, int threshold, Constraint_list *CL)
{
  int a,g, nseq, n_groups;
  nseq=(CL->S)->nseq;

  g=get_best_group(used, CL);

  used[g][0]=1;



  while ( seq_aln_thr2 (A, used, threshold,g, CL)!=0)
    {
      g=get_best_group (used, CL);
      used[g][0]=1;
    }

  for (n_groups=0,a=0; a< nseq; a++)
    if ( used[a][1]!=0)
      {
	n_groups++;
	used[a][0]=0;
      }
  return n_groups;
}


int seq_aln_thr2( Alignment*A, int **used, int threshold, int g, Constraint_list *CL)
{
  int a, b,c,d;
  int ns[2], *ls[2];
  int nseq, n_members;
  double sim;

  n_members=0;

  nseq=((CL->S)->nseq);
  used[g][0]=1;
  ns[0]=used[g][1];
  ls[0]=used[g]+2;

  for ( a=0; a< nseq; a++)
    {
      if (used[a][0]!=0);
      else
	{
	  ns[1]=used[a][1];
	  ls[1]=used[a]+2;

	  ungap_sub_aln (A, ns[0], ls[0]);
	  ungap_sub_aln (A, ns[1], ls[1]);

	  A->score_aln=pair_wise (A, ns, ls,CL);

	  for (sim=0,b=0; b<ns[0]; b++)
	    {
	      for (c=0; c<ns[1]; c++)
		{
		  sim+=generic_get_seq_sim (A->seq_al[ls[0][b]], A->seq_al[ls[1][c]], NULL,"idmat_sim2");
		}
	    }
	  sim/=(double)(ns[0]*ns[1]);
	  if (sim>=threshold)
	    {

	      used[g][1]+=ns[1];
	      for (d=0; d<ns[1]; d++)
		ls[0][ns[0]++]=ls[1][d];
	      used[a][0]=-1;
	      used[a][1]=0;
	      b=ns[0];c=ns[1];
	      n_members++;
	    }

	}
    }

  if (n_members>0)used[g][0]=-1;
  return n_members;
}
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*            Alignment Methods                                             */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

Alignment * tsp_aln (Alignment *A, Constraint_list *CL, Sequence *S)
{
  int a, b   ;
  int **  distances;
  int *ns, **ls;
  int **used;

  A=reorder_aln (A, (CL->S)->name,(CL->S)->nseq);
  ns=vcalloc (2, sizeof (int));
  ls=declare_int (2, (CL->S)->nseq);
  used=declare_int ( A->nseq, 2);


  CL->DM=cl2distance_matrix (CL, NOALN,NULL,NULL,0);
  distances=declare_int (A->nseq+1, A->nseq+1);
  distances=duplicate_int ( (CL->DM)->score_similarity_matrix, -1, -1);

  for (a=0; a< A->nseq; a++)
    {
      used[a][0]=a;
      for (b=0; b< A->nseq; b++)
	{
	  used[a][1]+=distances[a][b];
	}
    }

  sort_int_inv (used,2,1,0,(CL->S)->nseq-1);

  ls[0][ns[0]++]=used[0][0];
  ns[1]=1;

  for (a=1; a< S->nseq; a++)
    {
      fprintf ( stderr, "\n%s %d", (CL->S)->name[used[a][0]], used[a][1]);
      ls[1][0]=used[a][0];
      pair_wise ( A,ns,ls, CL);
      ls[0][ns[0]++]=used[a][0];
    }

  A->nseq=(CL->S)->nseq;
  return A;

}

Alignment *stack_progressive_aln(Alignment *A, Constraint_list *CL, int gop, int gep)
    {
    int a,n;
    int**group_list;
    int *n_groups;
    char dp_mode[100];


    sprintf ( dp_mode, "%s", CL->dp_mode);
    sprintf (CL->dp_mode, "gotoh_pair_wise");

    n_groups=vcalloc ( 2, sizeof (int));
    group_list=declare_int ( 2, A->nseq);

    n=A->nseq;

    for ( a=0; a<n; a++)ungap(A->seq_al[a]);
    for ( a=1; a<n; a++)
        {
	n_groups[0]=a;
	n_groups[1]=1;
	group_list[0][a-1]=a-1;
	group_list[1][0]  =a;

	pair_wise ( A,n_groups, group_list, CL);
	fprintf ( stderr, "\n\t[%d]->[%d]", a,(int)strlen ( A->seq_al[0]));
	}
    fprintf (stderr, "\n");
    vfree(n_groups);
    free_int ( group_list, -1);
    sprintf (CL->dp_mode, "%s",dp_mode);

    return A;
    }
Alignment *realign_aln_clust ( Alignment*A, Constraint_list *CL);
Alignment *realign_aln_random_bipart_n ( Alignment*A, Constraint_list *CL, int n);
Alignment *iterate_aln ( Alignment*A, int nit, Constraint_list *CL)
{
  int it;
  int mode=1;
  int score, iscore, delta;
  fprintf ( CL->local_stderr, "Iterated Refinement: %d cycles START: score= %d\n", nit,iscore=aln2ecl_raw_score (A, CL) );


  if ( nit==-1)nit=A->nseq*2;
  if ( A->len_aln==0)A=very_fast_aln (A, A->nseq, CL);
  A=reorder_aln (A,(CL->S)->name, A->nseq);

  for (it=0; it< nit; it++)
    {
      //CL->local_stderr=output_completion (CL->local_stderr,it, nit,1, "");
      if (mode==0)A=realign_aln (A, CL);
      else if (mode ==1)A=realign_aln_random_bipart (A, CL);
      else if (mode ==2)A=realign_aln_clust (A, CL);
      else if (mode ==3)A=realign_aln_random_bipart_n (A, CL,2);


      score=aln2ecl_raw_score (A, CL);
      delta=iscore-score;
      fprintf (CL->local_stderr, "\n\tIteration Cycle: %d Score=%d Improvement= %d", it+1,score, delta);
    }
  fprintf ( CL->local_stderr, "\nIterated Refinement: Completed Improvement=%d\n", delta);
  return A;
}

int get_next_best (int seq, int nseq, int *used, int **dm);
int get_next_best (int seq, int nseq, int *used, int **dm)
{
  int a,set, d, bd, bseq;

  for (set=0,a=0; a< nseq; a++)
    {
      if (used[a] || seq==a)continue;
      d=dm[seq][a];
      if (set==0 || d>bd)
	{
	  bseq=a;
	  bd=d;
	  set=1;
	}
    }
  return bseq;
}
Alignment  * full_sorted_aln (Alignment *A, Constraint_list *CL)
{
  int a,b;
  A=sorted_aln_seq (0, A, CL);
  print_aln(A);
  for (a=1; a<A->nseq; a++)
    {
      A=A->A=copy_aln (A, NULL);
      for (b=0; b<A->nseq; b++)ungap(A->seq_al[b]);
      A=sorted_aln_seq (a, A, CL);
      print_aln(A);
    }
  return A;
}
Alignment * sorted_aln (Alignment *A, Constraint_list *CL)
{
  return sorted_aln_seq (-1, A, CL);
}
Alignment * sorted_aln_seq (int new_seq, Alignment *A, Constraint_list *CL)
{
  int a, b=0, nseq;
  int *ns, **ls, **score, *used, **dm;
  int old_seq;

  dm=(CL->DM)->score_similarity_matrix;
  nseq=(CL->S)->nseq;
  score=declare_int (nseq, 3);
  used=vcalloc (nseq, sizeof (int));
  ls=declare_int (2, nseq);
  ns=vcalloc (2, sizeof (int));


  if ( new_seq==-1)
    {
      for (a=0; a<nseq; a++)
	{
	  score[a][0]=a;
	  score[a][1]=b;
	  for ( b=0; b<nseq; b++)
	    score[a][2]+=dm[a][b];
	}
      sort_int ( score,3, 2, 0, nseq-1);
      old_seq=new_seq=score[nseq-1][0];
    }

  for (a=1; a< nseq; a++)
    {
      used[new_seq]=1;
      ls[0][ns[0]++]=new_seq;
      ns[1]=1;
      ls[1][0]=get_next_best(new_seq,nseq, used,dm);
      old_seq=new_seq;
      new_seq=ls[1][0];

      A->score_aln=pair_wise (A, ns, ls,CL);

    }
  return A;
}

Alignment * ungap_aln4tree (Alignment *A);
Alignment * ungap_aln4tree (Alignment *A)
{
  int t, n, max_sim, sim;
  Alignment *B;



  n=35;
  max_sim=60;

  t=A->len_aln/10;

  B=copy_aln (A, NULL);
  B=ungap_aln_n(B, n);
  return B;

  sim=aln2sim (B, "idmat");
  while (B->len_aln<t && sim>max_sim && n>0)
    {
      n-=10;
      B=copy_aln (A, B);
      B=ungap_aln_n(B, n);
      sim=aln2sim (B, "idmat");
    }
  if ( B->len_aln<t && sim>max_sim)B=copy_aln (A, B);
  return B;
}



Alignment * iterative_tree_aln (Alignment *A,int n, Constraint_list *CL)
{
  NT_node **T=NULL;
  int a;

  T=make_tree (A, CL, CL->gop, CL->gep,CL->S,NULL, 1);
  tree_aln ((T[3][0])->left,(T[3][0])->right,A,(CL->S)->nseq, CL);
  for ( a=0; a< n; a++)
    {

      Alignment *B;

      B=copy_aln (A, NULL);
      B=ungap_aln_n (B, 20);
      sprintf ( CL->distance_matrix_mode, "aln");

      CL->DM=cl2distance_matrix ( CL,B,NULL,NULL, 1);
      free_aln (B);

      degap_aln (A);
      T=make_tree (A, CL, CL->gop, CL->gep,CL->S,NULL, 1);

      tree_aln ((T[3][0])->left,(T[3][0])->right,A,(CL->S)->nseq, CL);
    }
  return A;
}

Alignment *profile_aln (Alignment *A, Constraint_list *CL)
{
  int a,nseq,nseq2;
  int **ls, *ns;
  nseq=A->nseq;
  nseq2=2*nseq;
  ls=declare_int (2, nseq2);
  ns=vcalloc (2, sizeof (int));

  A=realloc_aln2(A,nseq2, A->len_aln);
  for (a=0; a< nseq; a++)
    ls[0][ns[0]++]=a;
  for ( a=0; a<nseq;a++)
    {
      sprintf (A->seq_al[a+nseq], "%s", (CL->S)->seq[a]);
      sprintf (A->name[a+nseq], "%s", (CL->S)->name[a]);
      A->order[a+nseq][0]=a;
    }

  ns[1]=1;
  for (a=0; a<nseq; a++)
    {

      ls[1][0]=a+nseq;
      A->score_aln=pair_wise (A, ns, ls,CL);

      ls[0][ns[0]++]=a+nseq;
    }
  for (a=0; a< nseq; a++)
    {
      sprintf (A->seq_al[a], "%s", A->seq_al[a+nseq]);
    }
  A->nseq=nseq;
  return A;
}

Alignment * iterative_aln ( Alignment*A, int n,Constraint_list *CL)
{
  int *ns,**ls, **score, **dm;
  int a,b, nseq, max;
  ls=declare_int (2, A->nseq);
  ns=vcalloc (2, sizeof (int));
  ls[0][ns[0]++]=0;





  nseq=(CL->S)->nseq;
  score=declare_int (nseq,2);
  dm=(CL->DM)->score_similarity_matrix;
  for (a=0; a<nseq; a++)
    {
      score[a][0]=a;
      for ( b=0; b<nseq; b++)
	score[a][1]+=dm[a][b];
      score[a][1]/=nseq;
    }
  sort_int ( score,2, 1, 0, nseq-1);

  max=20;
  for (a=0; a<max; a++)
    {
      ns[0]=nseq-1;
      for (ns[0]=0,b=0; b<nseq; b++)
	if (b!=score[a][0])ls[0][ns[0]++]=b;

      fprintf (stderr, "[%s %s %d]",(CL->S)->name[score[a][0]],A->name[score[a][0]], score[a][1]);
      ns[1]=1;
      ls[1][0]=score[a][0];
      ungap_sub_aln ( A, ns[0], ls[0]);
      ungap_sub_aln ( A, ns[1], ls[1]);
      A->score_aln=pair_wise (A, ns, ls,CL);
      ls[0][ns[0]++]=a;
    }

  return A;
}
Alignment *simple_progressive_aln (Sequence *S, NT_node **T, Constraint_list *CL, char *mat)
{
  int a;
  Alignment *A;


  A=seq2aln (S, NULL, RM_GAP);

  if ( !CL)
    {

      CL=declare_constraint_list (S, NULL, NULL, 0, NULL, NULL);
      sprintf ( CL->dp_mode,   "myers_miller_pair_wise");
      sprintf ( CL->tree_mode, "nj");
      sprintf ( CL->distance_matrix_mode, "idscore");
      CL=choose_extension_mode ("matrix", CL);
      CL->gop=-10;
      CL->gep=-1;
      if (mat)CL->M=read_matrice (mat);
      CL->pw_parameters_set=1;
      CL->local_stderr=stderr;
    }

  if ( !T)T=make_tree (A, CL, CL->gop, CL->gep,S, NULL,MAXIMISE);
  for ( a=0; a< A->nseq; a++)ungap (A->seq_al[a]);

  tree_aln ((T[3][0])->left,(T[3][0])->right,A,(CL->S)->nseq, CL);
  A=reorder_aln ( A,A->tree_order,A->nseq);

  return A;
}

Alignment *very_fast_aln ( Alignment*A, int nseq, Constraint_list *CL)
{
char command[10000];
char *tmp_seq;
char *tmp_aln;
FILE *fp;

if ( CL && CL->local_stderr)fp=CL->local_stderr;
else fp=stderr;

 fprintf (fp, "\n[Computation of an Approximate MSA...");
 tmp_seq= vtmpnam (NULL);
 tmp_aln= vtmpnam (NULL);
 output_fasta_seq ((tmp_seq=vtmpnam (NULL)), A);
 sprintf ( command, "t_coffee -infile=%s -special_mode quickaln -outfile=%s %s -outorder=input", tmp_seq, tmp_aln, TO_NULL_DEVICE);
 my_system ( command);
 A->nseq=0;
 A=main_read_aln (tmp_aln,A);
 fprintf (fp, "]\n");
 return A;
}

static NT_node* SNL;
NT_node* tree_aln ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL)
{
  if ( strm ((CL->TC)->use_seqan, "NO"))return local_tree_aln (LT, RT, A, nseq, CL);
  else return seqan_tree_aln (LT, RT, A, nseq, CL);
}

NT_node* seqan_tree_aln ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL)
  {


    Alignment *B;


    char *tree, *lib, *seq, *new_aln;


    //Output tree
    tree=vtmpnam (NULL);
    print_newick_tree (LT->parent, tree);


    //Output seq
    main_output_fasta_seq (seq=vtmpnam (NULL),B=seq2aln (CL->S,NULL,RM_GAP), NO_HEADER);
    free_aln (B);

    //Output lib
    new_aln=vtmpnam (NULL);
    vfclose (save_constraint_list ( CL, 0, CL->ne,lib=vtmpnam(NULL), NULL, "ascii",CL->S));

    fprintf (CL->local_stderr, "\n********* USE EXTERNAL ALIGNER: START:\n\tCOMMAND: %s -lib %s -seq %s -usetree %s -outfile %s\n", (CL->TC)->use_seqan,lib, seq, tree, new_aln);
    printf_system ( "%s -lib %s -seq %s -usetree %s -outfile %s", (CL->TC)->use_seqan,lib, seq, tree, new_aln);
    fprintf (CL->local_stderr, "\n********* USE EXTERNAL ALIGNER: END\n");


    main_read_aln (new_aln, A);
    return tree2ao (LT,RT, A, A->nseq, CL);



  }
NT_node rec_local_tree_aln ( NT_node P, Alignment*A, Constraint_list *CL, int print);
NT_node* local_tree_aln ( NT_node l, NT_node r, Alignment*A,int nseq, Constraint_list *CL)
{
  int a;
  NT_node P, *NL;
  int **min=NULL;

  if (!r && !l) return NULL;
  else if (!r)P=l;
  else if (!l)P=r;
  else P=r->parent;

  fprintf ( CL->local_stderr, "\nPROGRESSIVE_ALIGNMENT [Tree Based]\n");

  //1: make sure the Alignment and the Sequences are labeled the same way
  if (CL->translation)vfree (CL->translation);
  CL->translation=vcalloc ( (CL->S)->nseq, sizeof (int));
  for ( a=0; a< (CL->S)->nseq; a++)
    CL->translation[a]=name_is_in_list ( (CL->S)->name[a], (CL->S)->name, (CL->S)->nseq, MAXNAMES);
  A=reorder_aln (A, (CL->S)->name,(CL->S)->nseq);
  A->nseq=(CL->S)->nseq;

  //2 Make sure the tree is in the same order
  recode_tree (P, (CL->S));
  index_tree_node(P);
  initialize_scoring_scheme (CL);
  
  if ( get_nproc()>1 && strstr (CL->multi_thread, "msa"))
    {
      int max_fork;
      
      max_fork=get_nproc()/2;//number of nodes forked, one node =>two jobs
      tree2nnode (P);
      NL=tree2node_list (P, NULL);
      min=declare_int (P->node+1,3);
      for (a=0; a<=P->node; a++)
	{
	  NT_node N;
	  N=NL[a];
	  min[a][0]=a;
	  if (!N);
	  else if (N && N->nseq==1)min[a][1]=0;
	  else
	    {
	      min[a][1]=MIN(((N->left)->nseq),((N->right)->nseq))*A->nseq+MAX(((N->left)->nseq),((N->right)->nseq));//sort on min and break ties on max
	      min[a][2]=MIN(((N->left)->nseq),((N->right)->nseq));
	    }
	}
      sort_int_inv (min,3, 1, 0, P->node);
      for (a=0; a<=P->node && a<max_fork; a++)
	{
	  if (min[a][2]>1)(NL[min[a][0]])->fork=1;
	}
    }
  free_int (min, -1);
  rec_local_tree_aln (P, A,CL, 1);
  for (a=0; a<P->nseq; a++)sprintf (A->tree_order[a], "%s", (CL->S)->name[P->lseq[a]]);
  A->len_aln=strlen (A->seq_al[0]);

  fprintf ( CL->local_stderr, "\n\n");

  return NULL;
}

NT_node rec_local_tree_aln ( NT_node P, Alignment*A, Constraint_list *CL,int print)
{
  NT_node R,L;
  int score;


  if (!P || P->nseq==1) return NULL;
  R=P->right;L=P->left;

  if (P->fork )
    {
      int s, pid1, pid2;
      char *tmp1, *tmp2;
      tmp1=vtmpnam (NULL);
      tmp2=vtmpnam (NULL);

      pid1=vvfork(NULL);
      if (pid1==0)
	{
	  if (print==1)
	    if (L->nseq>R->nseq)print=1;

	  initiate_vtmpnam (NULL);
	  rec_local_tree_aln (L, A, CL, print);
	  dump_msa (tmp1,A, L->nseq, L->lseq);
	  myexit (EXIT_SUCCESS);
	}
      else
	{
	  pid2=vvfork(NULL);
	  if (pid2==0)
	    {
	      if (print==1)
		if (L->nseq>R->nseq)print=0;


	      initiate_vtmpnam (NULL);
	      rec_local_tree_aln (R, A, CL, print);
	      dump_msa (tmp2, A, R->nseq, R->lseq);
	      myexit (EXIT_SUCCESS);
	    }
	}
      vwaitpid (pid1, &s, 0);
      vwaitpid (pid2, &s, 0);


      undump_msa (A,tmp1);
      undump_msa (A,tmp2);
    }
  else
    {
      rec_local_tree_aln (L, A, CL, print);
      rec_local_tree_aln (R, A, CL, print);
    }

  P->score=A->score_aln=score=profile_pair_wise (A,L->nseq, L->lseq,R->nseq,R->lseq,CL);
  A->len_aln=strlen (A->seq_al[P->lseq[0]]);
  score=node2sub_aln_score (A, CL, CL->evaluate_mode,P);
  if (print)fprintf(CL->local_stderr, "\n\tGroup %4d: [Group %4d (%4d seq)] with [Group %4d (%4d seq)]-->[Score=%4d][Len=%5d][PID:%d]%s",P->index,R->index,R->nseq,L->index,L->nseq,score, A->len_aln,getpid(),(P->fork==1)?"[Forked]":"" );

  return P;
}



NT_node* tree2ao ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL)
    {
    int *n_s;
    int ** l_s;
    int a, b;

    static int n_groups_done, do_split=0;
    int  nseq2align=0;
    int *translation;


    NT_node P=NULL;




    if (n_groups_done==0)
       {
	 if (SNL)vfree(SNL);
	 SNL=vcalloc ( (CL->S)->nseq, sizeof (NT_node));

	 if (CL->translation)vfree(CL->translation);
	 CL->translation=vcalloc ( (CL->S)->nseq, sizeof (int));

	 for ( a=0; a< (CL->S)->nseq; a++)
	   CL->translation[a]=name_is_in_list ( (CL->S)->name[a], (CL->S)->name, (CL->S)->nseq, MAXNAMES);

	 n_groups_done=(CL->S)->nseq;
	 A=reorder_aln (A, (CL->S)->name,(CL->S)->nseq);
	 A->nseq=nseq;
       }

    translation=CL->translation;
    n_s=vcalloc (2, sizeof ( int));
    l_s=declare_int ( 2, nseq);


    if ( RT->parent !=LT->parent)fprintf ( stderr, "Tree Pb [FATAL:%s]", PROGRAM);
    else P=RT->parent;

    if ( LT->leaf==1 && RT->leaf==0)
      tree2ao ( RT->left, RT->right,A, nseq,CL);

    else if ( RT->leaf==1 && LT->leaf==0)
      tree2ao ( LT->left, LT->right,A,nseq,CL);

    else if (RT->leaf==0 && LT->leaf==0)
      {
	tree2ao ( LT->left, LT->right,A,nseq,CL);
	tree2ao ( RT->left, RT->right,A,nseq,CL);
      }

    if ( LT->leaf==1 && RT->leaf==1)
      {
	/*1 Identify the two groups of sequences to align*/

	nseq2align=LT->nseq+RT->nseq;
	n_s[0]=LT->nseq;
	for ( a=0; a< LT->nseq; a++)l_s[0][a]=translation[LT->lseq[a]];
	if ( LT->nseq==1)LT->group=l_s[0][0];

	n_s[1]=RT->nseq;
	for ( a=0; a< RT->nseq; a++)l_s[1][a]=translation[RT->lseq[a]];
	if ( RT->nseq==1)RT->group=l_s[1][0];


	P->group=n_groups_done++;

	if (nseq2align==nseq)
	  {
	    for (b=0, a=0; a< n_s[0]; a++, b++)sprintf ( A->tree_order[b],"%s", (CL->S)->name[l_s[0][a]]);
	    for (a=0; a< n_s[1]     ; a++, b++)sprintf ( A->tree_order[b], "%s",(CL->S)->name[l_s[1][a]]);
	    n_groups_done=0;
	  }
      }
    if (P->parent)P->leaf=1;
    if ( LT->isseq==0)LT->leaf=0;
    if ( RT->isseq==0)RT->leaf=0;

    if (RT->isseq){SNL[translation[RT->lseq[0]]]=RT;RT->score=100;}
    if (LT->isseq){SNL[translation[LT->lseq[0]]]=LT;LT->score=100;}

    do_split=split_condition (nseq2align,A->score_aln,CL);
    if (CL->split && do_split)
      {

	for (a=0; a< P->nseq; a++)SNL[CL->translation[P->lseq[a]]]=NULL;
	SNL[CL->translation[RT->lseq[0]]]=P;

      }

    vfree ( n_s);
    free_int ( l_s, 2);
    return SNL;

    }

NT_node* tree_realn ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL)
    {
    int *n_s;
    int ** l_s;
    int a, b;
    int score;
    static int n_groups_done;
    int nseq2align=0;
    int *translation;


    NT_node P=NULL;




    if (n_groups_done==0)
       {
	 if (SNL)vfree(SNL);
	 SNL=vcalloc ( (CL->S)->nseq, sizeof (NT_node));

	 if (CL->translation)vfree(CL->translation);
	 CL->translation=vcalloc ( (CL->S)->nseq, sizeof (int));

	 for ( a=0; a< (CL->S)->nseq; a++)
	   CL->translation[a]=name_is_in_list ( (CL->S)->name[a], (CL->S)->name, (CL->S)->nseq, MAXNAMES);
	 if (nseq>2)fprintf ( CL->local_stderr, "\nPROGRESSIVE_ALIGNMENT [Tree Based]\n");
	 else fprintf ( CL->local_stderr, "\nPAIRWISE_ALIGNMENT [No Tree]\n");
	 n_groups_done=(CL->S)->nseq;
	 A=reorder_aln (A, (CL->S)->name,(CL->S)->nseq);
	 A->nseq=nseq;
       }

    translation=CL->translation;
    n_s=vcalloc (2, sizeof ( int));
    l_s=declare_int ( 2, nseq);


    if ( nseq==2)
       {
       n_s[0]=n_s[1]=1;
       l_s[0][0]=name_is_in_list ((CL->S)->name[0],(CL->S)->name, (CL->S)->nseq, MAXNAMES);
       l_s[1][0]=name_is_in_list ((CL->S)->name[1],(CL->S)->name, (CL->S)->nseq, MAXNAMES);
       A->score_aln=score=pair_wise (A, n_s, l_s,CL);

       vfree ( n_s);
       free_int ( l_s, 2);
       return SNL;
       }
    else
       {
       if ( RT->parent !=LT->parent)fprintf ( stderr, "Tree Pb [FATAL:%s]", PROGRAM);
       else P=RT->parent;

       if ( LT->leaf==1 && RT->leaf==0)
	   tree_realn ( RT->left, RT->right,A, nseq,CL);

       else if ( RT->leaf==1 && LT->leaf==0)
	   tree_realn ( LT->left, LT->right,A,nseq,CL);

       else if (RT->leaf==0 && LT->leaf==0)
          {
	  tree_realn ( LT->left, LT->right,A,nseq,CL);
	  tree_realn ( RT->left, RT->right,A,nseq,CL);
	  }

       if ( LT->leaf==1 && RT->leaf==1 && (RT->nseq+LT->nseq)<nseq)
          {
	  /*1 Identify the two groups of sequences to align*/
	    int *list, s, id1, id2;
	    list=vcalloc (nseq, sizeof (int));
	    for (a=0; a<LT->nseq; a++)
	      {
		s=translation[LT->lseq[a]];
		list[s]=1;
	      }
	    for (a=0; a<RT->nseq; a++)
	      {
		s=translation[RT->lseq[a]];
		list[s]=1;
	      }
	    for (a=0; a<nseq; a++)
	      {
		s=list[a];
		l_s[s][n_s[s]++]=a;
	      }

	    vfree (list);

	    id1=sub_aln2sim (A, n_s, l_s, "idmat_sim");


	    ungap_sub_aln (A, n_s[0],l_s[0]);
	    ungap_sub_aln (A, n_s[1],l_s[1]);
	    P->score=A->score_aln=score=pair_wise (A, n_s, l_s,CL);
	    id2=sub_aln2sim (A, n_s, l_s, "idmat_sim");




	    if (nseq2align==nseq)
	      {
		for (b=0, a=0; a< n_s[0]; a++, b++)sprintf ( A->tree_order[b],"%s", (CL->S)->name[l_s[0][a]]);
		for (a=0; a< n_s[1]     ; a++, b++)sprintf ( A->tree_order[b], "%s",(CL->S)->name[l_s[1][a]]);
		n_groups_done=0;
	      }
	  }
       if (P->parent)P->leaf=1;
       //Recycle the tree
       if ( LT->isseq==0)LT->leaf=0;
       if ( RT->isseq==0)RT->leaf=0;

       if (RT->isseq){SNL[translation[RT->lseq[0]]]=RT;RT->score=100;}
       if (LT->isseq){SNL[translation[LT->lseq[0]]]=LT;LT->score=100;}

       vfree ( n_s);
       free_int ( l_s, 2);
       return SNL;
       }


    }



Alignment* profile_tree_aln ( NT_node P,Alignment*A,Constraint_list *CL, int threshold)
{
  int *ns, **ls, a, sim;
  NT_node LT, RT, D, UD;
  Alignment *F;
  static NT_node R;
  static int n_groups_done;


  //first pass
  //Sequences must be in the same order as the tree sequences
   if (!P->parent)
    {
      R=P;
      n_groups_done=P->nseq+1;
    }

  LT=P->left;
  RT=P->right;

  if (LT->leaf==0)A=delayed_tree_aln1 (LT, A,CL, threshold);
  if (RT->leaf==0)A=delayed_tree_aln1 (RT, A,CL, threshold);

  ns=vcalloc (2, sizeof (int));
  ls=declare_int ( 2,R->nseq);

  if ( LT->nseq==1)
    {
      ls[0][ns[0]++]=LT->lseq[0];
      LT->group=ls[0][0]+1;
    }
  else
    node2seq_list (LT,&ns[0], ls[0]);

  if ( RT->nseq==1)
    {
      ls[1][ns[1]++]=RT->lseq[0];
      RT->group=ls[1][0]+1;
    }
  else
    node2seq_list (RT,&ns[1], ls[1]);


  P->group=++n_groups_done;
  fprintf (CL->local_stderr, "\n\tGroup %4d: [Group %4d (%4d seq)] with [Group %4d (%4d seq)]-->",P->group,RT->group, ns[1],LT->group, ns[0]);

  P->score=A->score_aln=pair_wise (A, ns, ls,CL);
  sim=sub_aln2sim(A, ns, ls, "idmat_sim1");

  if ( sim<threshold)
    {
      UD=(ns[0]<=ns[1])?RT:LT;
      D= (ns[0]<=ns[1])?LT:RT;

      UD->aligned=1;
      D->aligned=0;

      fprintf (CL->local_stderr,  "[Delayed (Sim=%4d). Kept Group %4d]",sim,UD->group);


      ungap_sub_aln (A, ns[0],ls[0]);
      ungap_sub_aln (A, ns[1],ls[1]);
      A->nseq=MAX(ns[0],ns[1]);

      F=A;
      while (F->A)F=F->A;
      F->A=main_read_aln (output_fasta_sub_aln (NULL, A, ns[(D==LT)?0:1], ls[(D==LT)?0:1]), NULL);
      if ( P==R)
	{
	  F=F->A;
	  F->A=main_read_aln (output_fasta_sub_aln (NULL, A, ns[(D==LT)?1:0], ls[(D==LT)?1:0]), NULL);
	}
      if (F->A==NULL)
	{
	  printf_exit (EXIT_FAILURE, stderr, "\nError: Empty group");
	}
    }
  else
    {
      LT->aligned=1; RT->aligned=1;
      fprintf (CL->local_stderr, "[Score=%4d][Len=%5d]",sub_aln2sub_aln_score (A, CL, CL->evaluate_mode,ns, ls), (int)strlen ( A->seq_al[ls[0][0]]));
      A->nseq=ns[0]+ns[1];
      if (P==R)
	{
	  F=A;
	  while (F->A)F=F->A;
	  F->A=main_read_aln (output_fasta_sub_aln2 (NULL, A, ns, ls), NULL);
	}
    }
  P->nseq=0;
  for (a=0; a<LT->nseq;a++)P->lseq[P->nseq++]=LT->lseq[a];
  for (a=0; a<RT->nseq;a++)P->lseq[P->nseq++]=RT->lseq[a];

  P->aligned=1;

  vfree ( ns);
  free_int ( ls,-1);
  return A;
}////////////////////////////////////////////////////////////////////////////////////////
//
//                               Frame Tree Aln
//
////////////////////////////////////////////////////////////////////////////////////////

//Alignment *frame_tree_aln (Alignment *A, Constraint_list *CL)
//{


////////////////////////////////////////////////////////////////////////////////////////
//
//                               Delayed Tree Aln
//
////////////////////////////////////////////////////////////////////////////////////////
int delayed_pair_wise (Alignment *A, int *ns, int **ls,Constraint_list *CL);
NT_node* delayed_tree_aln_mode1 ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL);
NT_node* delayed_tree_aln_mode2 ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL);
int paint_nodes2aligned ( NT_node P,char **list, int n);

int reset_visited_nodes ( NT_node P);
int reset_visited_nodes2 ( NT_node P);
Alignment * make_delayed_tree_aln (Alignment *A,int n, Constraint_list *CL)
{
  NT_node **T=NULL;
  int a;

  T=make_tree (A, CL, CL->gop, CL->gep,CL->S,NULL, 1);
  delayed_tree_aln_mode1 ((T[3][0])->left,(T[3][0])->right,A,(CL->S)->nseq, CL);

  for ( a=0; a< n; a++)
    {

      sprintf ( CL->distance_matrix_mode, "aln");
      CL->DM=cl2distance_matrix ( CL,A,NULL,NULL, 1);
      degap_aln (A);
      T=make_tree (A, CL, CL->gop, CL->gep,CL->S,NULL, 1);
      delayed_tree_aln_mode1 ((T[3][0])->left,(T[3][0])->right,A,(CL->S)->nseq, CL);
    }

  return A;
}
NT_node* delayed_tree_aln_mode1 ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL)
{
  NT_node P;



  P=LT->parent;P->nseq=nseq;
  paint_nodes2aligned (P, NULL, 0);

  A=delayed_tree_aln1 (P, A, CL,50);
  A=delayed_tree_aln2 (P, A, CL, 0);
  return NULL;
}

NT_node* delayed_tree_aln_mode2 ( NT_node LT, NT_node RT, Alignment*A, int nseq, Constraint_list *CL)
{
  NT_node P;

  int thr=50;

  P=LT->parent;P->nseq=nseq;

  A=delayed_tree_aln1 (P, A, CL,thr);
  thr-=10;
  while (thr>=0)
    {
      A=delayed_tree_aln2 (P, A, CL, thr);
      thr-=10;
    }
  return NULL;
}

Alignment* delayed_tree_aln1 ( NT_node P,Alignment*A,Constraint_list *CL, int threshold)
{
  int *ns, **ls, a, sim;
  NT_node LT, RT, D, UD;

  static NT_node R;
  static int n_groups_done;


  //first pass
  //Sequences must be in the same order as the tree sequences
   if (!P->parent)
    {
      R=P;
      n_groups_done=P->nseq+1;
    }

  LT=P->left;
  RT=P->right;

  if (LT->leaf==0)A=delayed_tree_aln1 (LT, A,CL, threshold);
  if (RT->leaf==0)A=delayed_tree_aln1 (RT, A,CL, threshold);

  ns=vcalloc (2, sizeof (int));
  ls=declare_int ( 2,R->nseq);


  node2seq_list (LT,&ns[0], ls[0]);
  if ( LT->nseq==1)LT->group=LT->lseq[0]+1;

  node2seq_list (RT,&ns[1], ls[1]);
  if ( RT->nseq==1)RT->group=RT->lseq[0]+1;


  P->group=++n_groups_done;


  if ( ns[0]==0 || ns[1]==0)
    {
      fprintf (CL->local_stderr, "\n\tF-Group %4d: [Group %4d (%4d seq)] with [Group %4d (%4d seq)]-->Skipped",P->group,RT->group, ns[1],LT->group, ns[0]);

      LT->aligned=(ns[0]==0)?0:1;
      RT->aligned=(ns[1]==0)?0:1;
    }
  else
    {
      fprintf (CL->local_stderr, "\n\tF-Group %4d: [Group %4d (%4d seq)] with [Group %4d (%4d seq)]-->",P->group,RT->group, ns[1],LT->group, ns[0]);
      P->score=A->score_aln=pair_wise (A, ns, ls,CL);
      sim=sub_aln2max_sim(A, ns, ls, "idmat_sim1");


      if ( sim<threshold)
	{
	  UD=(ns[0]<=ns[1])?RT:LT;
	  D= (ns[0]<=ns[1])?LT:RT;

	  UD->aligned=1;
	  D->aligned=0;

	  fprintf (CL->local_stderr,  "[Delayed (Sim=%4d). Kept Group %4d]",sim,UD->group);

	  ungap_sub_aln (A, ns[0],ls[0]);
	  ungap_sub_aln (A, ns[1],ls[1]);
	  A->nseq=MAX(ns[0],ns[1]);
	}
      else
	{
	  LT->aligned=1; RT->aligned=1;
	  fprintf (CL->local_stderr, "[Score=%4d][Len=%5d]",sub_aln2sub_aln_score (A, CL, CL->evaluate_mode,ns, ls), (int)strlen ( A->seq_al[ls[0][0]]));
	  A->nseq=ns[0]+ns[1];
	}
      P->nseq=0;
      for (a=0; a<LT->nseq;a++)P->lseq[P->nseq++]=LT->lseq[a];
      for (a=0; a<RT->nseq;a++)P->lseq[P->nseq++]=RT->lseq[a];

      P->aligned=1;
    }
  vfree ( ns);
  free_int ( ls,-1);
  return A;
}

Alignment* delayed_tree_aln2 ( NT_node P,Alignment*A,Constraint_list *CL, int thr)
{

  NT_node LT, RT, D;

  static NT_node R;


  LT=P->left;
  RT=P->right;
  if (!P->parent)
    {
      R=P;
      fprintf (CL->local_stderr, "\n");
    }
  if (!LT->aligned && !RT->aligned)
    {
      printf_exit (EXIT_FAILURE, stderr, "ERROR: Unresolved Node On Groups %d  [FATAL:%s]\n", P->group,PROGRAM);
    }
  else if (!LT->aligned || !RT->aligned)
    {
      int *ns, **ls, sim;
      ns=vcalloc (2, sizeof (int));
      ls=declare_int (2, R->nseq);

      node2seq_list (R,&ns[0], ls[0]);

      D=(!LT->aligned)?LT:RT;
      D->aligned=1;
      node2seq_list (D,&ns[1], ls[1]);

      fprintf (CL->local_stderr, "\tS-Delayed Group %4d: [Group %4d (%4d seq)] with [Group %4d (%4d seq)]-->",P->group,D->group, ns[1],R->group, ns[0]);
      P->score=A->score_aln=pair_wise (A, ns, ls,CL);
      sim=sub_aln2max_sim(A, ns, ls, "idmat_sim1");
      if (sim<thr)
	{
	  fprintf (CL->local_stderr, " [Further Delayed]\n");
	  ungap_sub_aln (A, ns[0],ls[0]);
	  ungap_sub_aln (A, ns[1],ls[1]);
	  D->aligned=0;
	}
      else
	{
	  fprintf (CL->local_stderr, "[Score=%4d][Len=%5d][thr=%d]\n",sub_aln2sub_aln_score (A, CL, CL->evaluate_mode,ns, ls), (int)strlen ( A->seq_al[ls[0][0]]), thr);
	  D->aligned=1;
	}
      vfree (ns);free_int (ls, -1);
    }
  else
    {
      ;
    }

  if (LT->leaf==0)A=delayed_tree_aln2 (LT, A,CL, thr);
  if (RT->leaf==0)A=delayed_tree_aln2 (RT, A,CL, thr);

  return A;
}

int delayed_pair_wise (Alignment *A, int *ns, int **ls,Constraint_list *CL)
{
  int s,s1, s2, a, b;
  int **sim;


  pair_wise (A, ns, ls, CL);

  sim=fast_aln2sim_list (A, "sim3", ns, ls);

  sort_int_inv ( sim,3, 2,0, ns[0]*ns[1]-1);

  for (a=0; a< 2; a++)
    for ( b=0; b< ns[a]; b++)
      A->order[ls[a][b]][4]=-1;

  for (a=0; a< 10 && sim[a][0]!=-1; a++)
    {
      s1=sim[a][0];
      s2=sim[a][1];
      A->order[s1][4]=0;
      A->order[s2][4]=0;
    }

  ungap_sub_aln (A, ns[0],ls[0]);
  ungap_sub_aln (A, ns[1],ls[1]);

  s=pair_wise (A, ns, ls, CL);

  for (a=0; a< 2; a++)
    for ( b=0; b< ns[a]; b++)
      A->order[ls[a][b]][4]=0;

  free_int (sim, -1);
  return s;
}

int node2seq_list2 (NT_node P, int *ns, int *ls)
{

  if ( !P || P->visited ) return ns[0];
  else P->visited=1;

  if ( P->isseq)
    {
      ls[ns[0]++]=P->lseq[0];
    }

  if (P->left   && (P->left) ->aligned)node2seq_list2 (P->left, ns,ls);
  if (P->right  && (P->right)->aligned)node2seq_list2 (P->right,ns,ls);
  if (P->aligned && P->parent)node2seq_list2 (P->parent,ns,ls);


  return ns[0];
}

int node2seq_list (NT_node P, int *ns, int *ls)
{

  if ( P->isseq && P->aligned)
    {
      ls[ns[0]++]=P->lseq[0];
    }
  else
    {
      if (P->left &&  (P->left) ->aligned)node2seq_list (P->left, ns,ls);
      if (P->right && (P->right)->aligned)node2seq_list (P->right,ns,ls);
    }
  return ns[0];
}
int paint_nodes2aligned ( NT_node P,char **list, int n)
{
  int r=0;
  if ( P->leaf)
    {
      if ( list==NULL)
	P->aligned=1;
      else if ( name_is_in_list ( P->name, list, n, 100)!=-1)
	P->aligned=1;
      else
	P->aligned=0;
      return P->aligned;
    }
  else
    {
      r+=paint_nodes2aligned (P->left, list, n);
      r+=paint_nodes2aligned (P->right, list, n);
    }
  return r;
}

int reset_visited_nodes ( NT_node P)
{
  while (P->parent)P=P->parent;
  return reset_visited_nodes2 (P);
}
int reset_visited_nodes2 ( NT_node P)
{
  int r=0;
  if (P->left)r+=reset_visited_nodes2(P->left);
  if (P->right)r+=reset_visited_nodes2(P->right);
  r+=P->visited;
  P->visited=0;
  return r;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//                               DPA_MSA
//
////////////////////////////////////////////////////////////////////////////////////////

Alignment* dpa_msa2 ( NT_node P,Alignment*A,Constraint_list *CL);
Alignment *dpa_align_node (NT_node P,Alignment*A,Constraint_list *CL);
char *node2profile_list (NT_node P,Alignment*A,Constraint_list *CL, char *list);
char * output_node_aln (NT_node P, Alignment *A, char *name);
int node2nleaf ( NT_node P);

Alignment* dpa_aln (Alignment*A,Constraint_list *CL)
{
  NT_node P;

  A=iterative_tree_aln (A,1, CL);
  P=make_root_tree (A, CL, CL->gop, CL->gep,CL->S,NULL, 1);
  degap_aln (A);
  while (!P->leaf)
    A=dpa_msa2(P, A, CL);
  return A;
}

int node2nleaf ( NT_node P)
{
  int n=0;
  if ( P->leaf) return 1;
  else
    {
      n+=node2nleaf ( P->left);
      n+=node2nleaf ( P->right);
    }
  return n;
}
Alignment* dpa_msa2 ( NT_node P,Alignment*A,Constraint_list *CL)
{
  int maxnseq=20;
  int n, n_l, n_r;
  n=node2nleaf (P);


  if ( n>maxnseq)
    {
        n_l=node2nleaf (P->left);
	n_r=node2nleaf (P->right);
	if (n_l>n_r)
	  {
	    return dpa_msa2 (P->left, A, CL);
	  }
	else
	  {
	    return dpa_msa2 (P->right, A, CL);
	  }
    }
  A=dpa_align_node (P, A, CL);
  P->leaf=1;
  return A;
}

Alignment *dpa_align_node (NT_node P,Alignment*A,Constraint_list *CL)
{

  char *list, *tmp_aln;
  int a, b;
  Alignment *B;


  list=vcalloc ( P->nseq*100, sizeof (char));
  list=node2profile_list (P,A, CL, list);

  printf_system ( "t_coffee -profile %s -outfile=%s -dp_mode gotoh_pair_wise_lgp -msa_mode iterative_tree_aln -quiet", list,tmp_aln=vtmpnam (NULL));
  B=main_read_aln (tmp_aln, NULL);
  A=realloc_aln (A, B->len_aln+1);
  for ( a=0; a< B->nseq; a++)
    if ( (b=name_is_in_list (B->name[a], A->name, A->nseq, 100))!=-1)
      sprintf (A->seq_al[b], "%s", B->seq_al[a]);
  A->len_aln=B->len_aln;
  free_aln (B);
  vfree (list);
  return A;
}
char *node2profile_list (NT_node P,Alignment*A,Constraint_list *CL, char *list)
{
  if (!P->leaf)
    {
      list=node2profile_list (P->left, A, CL, list);
      list=node2profile_list (P->right, A, CL, list);
    }
  else
    {

      list=strcatf (list," %s", output_node_aln (P, A, NULL));
      if ( !P->isseq)P->leaf=0;
    }
  return list;
}
char * output_node_aln (NT_node P, Alignment *A, char *name)
{
  FILE *fp;
  int a;
  if (name==NULL) name=vtmpnam (NULL);
  fp=vfopen (name, "w");

  for (a=0; a< P->nseq; a++)
    fprintf ( fp, ">%s\n%s", A->name[P->lseq[a]], A->seq_al[P->lseq[a]]);
  vfclose (fp);
  return name;
}
////////////////////////////////////////////////////////////////////////////////////////
//
//                               NEW_DPA_MSA
//
////////////////////////////////////////////////////////////////////////////////////////

Alignment * new_dpa_aln (Alignment *A,Constraint_list *CL)
{
  NT_node P;

  char *tmp_aln;
  char *list;

  A=make_delayed_tree_aln (A,1, CL);
  P=make_root_tree (A, CL, CL->gop, CL->gep,CL->S,NULL, 1);
  set_node_score (A, P, "idmat_sim");


  list=split_nodes_nseq (A,P,15, list=vcalloc (P->nseq*200, sizeof (char)));
  printf_system ( "t_coffee -profile %s -outfile=%s -dp_mode gotoh_pair_wise_lgp -msa_mode iterative_tree_aln", list,tmp_aln=vtmpnam (NULL));
  return main_read_aln (tmp_aln, NULL);
}

char *split_nodes_nseq  (Alignment *A, NT_node P, int nseq, char *list)
{
  int a,n;

  n=P->nseq;
  a=100;
  while ( n>=nseq)
    {
      a--;
      n=count_threshold_nodes (A, P, a);
    }

  return split_nodes_idmax (A, P, a,list);
}
char *split_nodes_idmax (Alignment *A, NT_node P, int t, char *list)
{
  if (P->isseq || P->score>=t)
    {
      list=strcatf (list," %s", output_node_aln (P, A, NULL));
    }
  else if ( P->score<t)
    {
      list=split_nodes_idmax (A, P->left,t, list);
      list=split_nodes_idmax (A, P->right,t,list);
    }
  return list;
}
int count_threshold_nodes (Alignment *A, NT_node P, int t)
{
  int s=0;

  if (P->isseq || P->score>=t)
    {
      s=1;
    }
  else if ( P->score<t)
    {
      s+=count_threshold_nodes (A, P->left,t);
      s+=count_threshold_nodes (A, P->right,t);
    }

  return s;
}
int set_node_score (Alignment *A, NT_node P, char *mode)
{
  int a;
  int ns[2], *ls[2];

  if (P->isseq) return 0;
  for (a=0; a<2; a++)
    {
      NT_node N;
      N=(a==0)?P->left:P->right;
      ns[a]=N->nseq;
      ls[a]=N->lseq;
    }
  P->score=sub_aln2max_sim(A, ns, ls,mode);
  set_node_score (A,P->left, mode);
  set_node_score (A,P->right, mode);
  return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////
int split_condition (int nseq, int score, Constraint_list *CL)
{
  int cond1=1, cond2=1;


  if ( CL->split_nseq_thres)cond1 =(nseq<=CL->split_nseq_thres)?1:0;
  if ( CL->split_score_thres)cond2=(score>=CL->split_score_thres)?1:0;

  return (cond1 && cond2);
}
int profile_pair_wise (Alignment *A, int n1, int *l1, int n2, int *l2, Constraint_list *CL)
{
  static int *ns;
  static int **ls;

  if (!ns)
    {
      ns=vcalloc (2, sizeof (int));
      ls=vcalloc (2, sizeof (int*));
    }
  ns[0]=n1;
  ls[0]=l1;
  ns[1]=n2;
  ls[1]=l2;
  return pair_wise (A, ns, ls, CL);
}
int pair_wise (Alignment *A, int*ns, int **l_s,Constraint_list *CL )
    {
	/*
	 CL->maximise
	 CL->gop;
	 CL->gep
	 CL->TG_MODE;
	*/
	int score;

	int glocal;
	Pwfunc function;



	/*Make sure evaluation functions update their cache if needed*/
	A=update_aln_random_tag (A);

	if (! CL->pw_parameters_set)
		   {
		       fprintf ( stderr, "\nERROR pw_parameters_set must be set in pair_wise [FATAL]\n" );crash("");
		   }


	function=get_pair_wise_function(CL->pair_wise,  CL->dp_mode,&glocal);
	if ( CL->get_dp_cost==NULL)CL->get_dp_cost=get_dp_cost;

	if (strlen ( A->seq_al[l_s[0][0]])==0 || strlen ( A->seq_al[l_s[1][0]])==0)
	  score=empty_pair_wise ( A, ns, l_s, CL, glocal);
	else
	  score=function ( A, ns, l_s, CL);

	return score;
    }

int empty_pair_wise ( Alignment *A, int *ns, int **l_s, Constraint_list *CL, int glocal)
{
  int n=0, a, b;
  int *l=NULL;
  char *string;
  int l0, l1, len;

  if ( glocal==GLOBAL)
    {
      l0=strlen (A->seq_al[l_s[0][0]]);
      l1=strlen (A->seq_al[l_s[1][0]]);
      len=MAX(l1,l0);

      if ( len==0)return 0;
      else if (l0>l1){n=ns[1];l=l_s[1];}
      else if (l0<l1){n=ns[0];l=l_s[0];}
      string=generate_null (len);
      for ( a=0; a< n; a++)
	sprintf ( A->seq_al[l[a]], "%s", string);
      A->score=A->score_aln=0;
      A->len_aln=len;
      vfree ( string);
      return 0;
    }
  else if ( glocal==LALIGN)
    {
      A->A=declare_aln (A->S);
      (A->A)->len_aln=0;
      for ( a=0; a< 2; a++)
	for ( b=0; b<ns[a]; b++)
	  A->seq_al[l_s[a][b]][0]='\0';
      (A->A)->score_aln=(A->A)->score=0;
      return 0;
    }
  else return 0;
}




Pwfunc get_pair_wise_function (Pwfunc pw,char *dp_mode, int *glocal)
  {
    /*Returns a function and a mode (Glogal, Local...)*/



    int a;
    static int npw;
    static Pwfunc *pwl;
    static char **dpl;
    static int *dps;

    /*The first time: initialize the list of pairwse functions*/
    if ( npw==0)
      {
	pwl=vcalloc ( 100, sizeof (Pwfunc));
	dpl=declare_char (100, 100);
	dps=vcalloc ( 100, sizeof (int));

	pwl[npw]=fasta_cdna_pair_wise;
	sprintf (dpl[npw], "fasta_cdna_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=cfasta_cdna_pair_wise;
	sprintf (dpl[npw], "cfasta_cdna_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=idscore_pair_wise;
	sprintf (dpl[npw], "idscore_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=gotoh_pair_wise;
	sprintf (dpl[npw], "gotoh_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=gotoh_pair_wise_lgp;
	sprintf (dpl[npw], "gotoh_pair_wise_lgp");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=gotoh_pair_wise_lgp_sticky;
	sprintf (dpl[npw], "gotoh_pair_wise_lgp_sticky");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=proba_pair_wise;
	sprintf (dpl[npw], "proba_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=biphasic_pair_wise;
	sprintf (dpl[npw], "biphasic_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=subop1_pair_wise;
	sprintf (dpl[npw], "subop1_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=subop2_pair_wise;
	sprintf (dpl[npw], "subop2_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=myers_miller_pair_wise;
	sprintf (dpl[npw], "myers_miller_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=test_pair_wise;
	sprintf (dpl[npw], "test_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=fasta_gotoh_pair_wise;
	sprintf (dpl[npw], "fasta_pair_wise");
	dps[npw]=GLOBAL;
	npw++;
	pwl[npw]=cfasta_gotoh_pair_wise;
	sprintf (dpl[npw], "cfasta_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=very_fast_gotoh_pair_wise;
	sprintf (dpl[npw], "very_fast_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=gotoh_pair_wise_sw;
	sprintf (dpl[npw], "gotoh_pair_wise_sw");
	dps[npw]=LOCAL;
	npw++;

	pwl[npw]=cfasta_gotoh_pair_wise_sw;
	sprintf (dpl[npw], "cfasta_sw_pair_wise");
	dps[npw]=LOCAL;
	npw++;

	pwl[npw]=gotoh_pair_wise_lalign;
	sprintf (dpl[npw], "gotoh_pair_wise_lalign");
	dps[npw]=LALIGN;
	npw++;

	pwl[npw]=sim_pair_wise_lalign;
	sprintf (dpl[npw], "sim_pair_wise_lalign");
	dps[npw]=LALIGN;
	npw++;

	pwl[npw]=domain_pair_wise;
	sprintf (dpl[npw], "domain_pair_wise");
	dps[npw]=MOCCA;
	npw++;

	pwl[npw]=gotoh_pair_wise;
	sprintf (dpl[npw], "ssec_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=ktup_pair_wise;
	sprintf (dpl[npw], "ktup_pair_wise");
	dps[npw]=LOCAL;
	npw++;

	pwl[npw]=precomputed_pair_wise;
	sprintf (dpl[npw], "precomputed_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=myers_miller_pair_wise;
	sprintf (dpl[npw], "default");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=viterbi_pair_wise;
	sprintf (dpl[npw], "viterbi_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=viterbiL_pair_wise;
	sprintf (dpl[npw], "viterbiL_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=viterbiD_pair_wise;
	sprintf (dpl[npw], "viterbiD_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=seq_viterbi_pair_wise;
	sprintf (dpl[npw], "seq_viterbi_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=pavie_pair_wise;
	sprintf (dpl[npw], "pavie_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=glocal_pair_wise;
	sprintf (dpl[npw], "glocal_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=linked_pair_wise;
	sprintf (dpl[npw], "linked_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	pwl[npw]=clinked_pair_wise;
	sprintf (dpl[npw], "clinked_pair_wise");
	dps[npw]=GLOBAL;
	npw++;

	/*
	pwl[npw]=viterbiDGL_pair_wise;
	sprintf (dpl[npw], "viterbiDGL_pair_wise");
	dps[npw]=GLOBAL;
	npw++;
	*/
      }

    for ( a=0; a< npw; a++)
      {
	if ( (dp_mode && strm (dpl[a], dp_mode)) || pwl[a]==pw)
	     {
	       pw=pwl[a];
	       if (dp_mode)sprintf (dp_mode,"%s", dpl[a]);
	       glocal[0]=dps[a];
	       return pw;
	     }
      }
    fprintf ( stderr, "\n[%s] is an unknown mode for dp_mode[FATAL]\n", dp_mode);
    crash ( "\n");
    return NULL;
  }


/*******************************************************************************/
/*                                                                             */
/*                                                                             */
/*	Util Functions                                                         */
/*                                                                             */
/*	                                                                       */
/*******************************************************************************/

char *build_consensus ( char *seq1, char *seq2, char *dp_mode)
        {
	Alignment *A;
	char *buf;
	int a;
	char c1, c2;
	static char *mat;


	if ( !mat) mat=vcalloc ( STRING, sizeof (char));


	A=align_two_sequences (seq1, seq2, strcpy(mat,"idmat"), 0, 0,dp_mode);
	buf=vcalloc ( A->len_aln+1, sizeof (char));

	for ( a=0; a< A->len_aln; a++)
	    {
		c1=A->seq_al[0][a];
		c2=A->seq_al[1][a];
		if (is_gap(c1) && is_gap(c2))buf[a]='-';
		else if (is_gap(c1))buf[a]=c2;
		else if (is_gap(c2))buf[a]=c1;
		else if (c1!=c2){vfree (buf);buf=NULL;free_aln(A);return NULL;}
		else buf[a]=c1;
	    }
	buf[a]='\0';
	free_sequence (free_aln (A), -1);
	return buf;
	}



#ifdef FASTAL

combine_profile () Comobine two profiles into one, using the edit sequence produce by the DP
  edit_sequence () insert the gaps using the

int fastal (int argv, char **arg)
{
  Sequence *S;
  int a, b;
  SeqHasch *H=NULL;
  int ktup=2;

  S=get_fasta_sequence (arg[1], NULL);



  for (a=0; a<S->nseq-1; a++)
    {
      for (b=a+1; b<S->nseq; b++)
	{
	  mat[b][a]=mat[a][b]addrand()%100;
	}
    }

  int_dist2nj_tree (s, S->name, S->nseq, tree_name);
  T=main_read_tree (BT);
  =fastal_tree_aln (T->L,T->R,S);
}




NT_node fastal_tree_aln ( NT_node P, Sequence *S)
{
  int score;


  if (!P || P->nseq==1) return NULL;
  R=P->right;L=P->left;

  fastal_tree_aln (P->left,S);
  fastal_tree_aln (P->right,S);
  fastal_pair_wise (P);
  return P;
}


NT_node fastal_pair_wise (NT_node P)
{
  //X- 1
  //-X 2
  //XX 3
  //-- 4

  tb=fastal_align_profile ((P->right)->prf, (P->left)->prf);

  l=strlen (tb);
  for (a=0; a< l; a++)
    {
      pr1=pr2=0;
      if (tb[a]== 1 || tb[a] ==3)pr1=1;
      if (tb[a]== 2 || tb[a] ==3)pr2=1;

      for (b=0; b<20; b++)
	P->prf[a][b]=((pr1==1)?(P->right)->prf[ppr1][b]:0) + ((pr2==1)?(P->left)->prf[ppr2][b]:0);
      ppr1+=pr1;
      ppr2+=pr2;
    }
  free_int ((P->left)->prf, -1);
  free_int ((P->right)->prf, -1);
}
#endif
