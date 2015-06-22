#include <iostream>
#include <Program.h>
#include <Basic_block.h>
#include <Function.h>


int main(int argc, char * argv[]){

	if (argc < 2) {
	  cout << "erreur : pas de fichier assembleur" << endl;
	}	  
	Program prog(argv[1]);
	Function* functmp;
	list <Function*> myfunc; 
	list <Basic_block*> myBB;

	cout<<"Le programme a "<<prog.size()<<" lignes\n"<<endl;

	prog.comput_function();
	cout<<"nombre de fonctions : "<<prog.nbr_func()<<endl;

	list<Function*>::iterator itfct;
	list<Basic_block*>::iterator itbb;
	Basic_block *bb;
	int i, j;
	list<int> frees;
	Dfg *d;
	Cfg *c;
	
	std::ostringstream *oss ;
	for(itfct=prog.function_list_begin(), i=0;
	    itfct!=prog.function_list_end(); itfct++, i++){
	   functmp=*itfct;
	    cout<<"------------Function DISPLAY----------\n" <<endl;
	    functmp->display();
	   functmp->comput_basic_block();
	   functmp->comput_label();
	   functmp->comput_succ_pred_BB();
            functmp->compute_dom();
	   
	   oss=new std::ostringstream;
	   
	   (*oss)<<"./tmp/func_"<<i<<".dot";
	   c=new Cfg(functmp->get_BB(0), functmp->nbr_BB());
	   c->restitution(NULL, oss->str());
	   
	   cout<<"========== Function "<<i<<"==========="<<endl;
	   cout<<"============================"<<endl;
	   
	   functmp ->compute_live_var();
	   j=0;
	   for(itbb=functmp->bb_list_begin(); 
	       itbb!=functmp->bb_list_end(); itbb++, j++){
	      bb=*itbb;
	      cout<<"----------BB "<<j<<"-----------"<<endl;
	      bb->display();
	      bb->link_instructions();
	      bb->comput_pred_succ_dep();
              bb->compute_def_liveout();
	      cout<<"---nb_cycles : "<<bb->nb_cycles()<<"-----------"<<endl;
	      
              d = new Dfg(bb);
	      cout<<"comput critical path"<<endl;
              oss=new std::ostringstream;
	   
              (*oss)<<"./tmp/1_dfg"<<j<<".dot";
              d->restitute(NULL, oss->str(), true);
	      d->comput_critical_path();
	      //cout<<"critical path "<<d->get_critical_path()<<endl;
	      
              
              
	      // liste de registres pour le renommage
	      // avec des registres pass�ees en param�tre 
	      /*
	      frees.clear();
	      for(int k=32; k<64; k++){
	      	 frees.push_back(k);
	      }
	      */
	      /* renommage en utilisant des registres n'existant pas */
	      
	      //  bb->reg_rename(&frees);

	      /* renommage utilisant les registres disponibles dans le bloc */
	      /* ne pas faire les 2 */
	      /* il faut recalculer les informations de vivacit� et de def-use 
	         pour pouvoir le faire 2 fois de suite !!
	      */
	      bb->reg_rename();
	      cout<<"----- apres renommage ------"<<endl;
	      bb->display();
	      // il faut annuler le calcul des d�pendances et le refaire
	      bb->reset_pred_succ_dep();
	      bb->comput_pred_succ_dep();
	      cout<<"---nb_cycles--"<<bb->nb_cycles()<<"-----------"<<endl;

	      d= new Dfg(bb);
	      cout<<"comput critical path"<<endl;
	      d->restitute(NULL, "./tmp/graph_dfg1.dot", true);
	      d->comput_critical_path();
	      cout<<"critical path "<<d->get_critical_path()<<endl;
	  
	      d->scheduling(false);
	      d->apply_scheduling();
	      cout<<"---- BB with new scheduling: -----"<<endl;
	      bb->display();
	     
	      cout<<"----nb_cycles--"<<bb->nb_cycles()<<"-----------"<<endl;
	     
	   }
           
           functmp->test();
	}
	    
	
	
	return 0;
	
}
