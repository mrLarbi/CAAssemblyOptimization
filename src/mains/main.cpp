#include <iostream>
#include <Instruction.h>
#include <Program.h>
#include <Directive.h>
#include <Label.h>
#include <fstream>


int main(int argc, char** argv){
      if (argc < 2) {
        cout << "erreur : pas de fichier assembleur" << endl;
      }	  
      Program prog(argv[1]);
      Function* functmp;
      list <Function*> myfunc; 
      list <Basic_block*> myBB;

      prog.comput_function();
      
      list<Function*>::iterator itfct;
      list<Basic_block*>::iterator itbb;
      Basic_block *bb;
      int i, j;
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
            cout<<"----------BB "<<j<<"-----------"<<endl<<endl;
            bb->display();
            bb->link_instructions();
            bb->comput_pred_succ_dep();
            bb->compute_def_liveout();
            cout<< endl <<"---nb_cycles : "<<bb->nb_cycles()<<"-----------"<<endl <<endl ;

            d = new Dfg(bb);
            oss=new std::ostringstream;

            (*oss)<<"./tmp/1_dfg"<<j<<".dot";
            d->restitute(NULL, oss->str(), true);
            d->comput_critical_path();

            bb->reg_rename();
            cout<<"----- apres renommage ------"<<endl << endl;
            bb->display();
            bb->reset_pred_succ_dep();
            bb->comput_pred_succ_dep();
            cout<< endl <<"---nb_cycles--"<<bb->nb_cycles()<<"-----------"<<endl << endl;

            d= new Dfg(bb);
            std::ostringstream *oss2 ;
            oss2=new std::ostringstream;
            (*oss2)<<"./tmp/graph_dfgRenom"<<j<<".dot";
            d->restitute(NULL, oss2->str(), true);
            d->comput_critical_path();

            d->scheduling(false);
            d->apply_scheduling();
            cout<<"---- BB with new scheduling: -----"<<endl << endl;
            bb->display();

            cout<< endl <<"----nb_cycles--"<<bb->nb_cycles()<<"-----------"<<endl << endl;

         }
      }



      return 0;

}
