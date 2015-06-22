#include <Basic_block.h>


//static
void Basic_block::show_dependances(Instruction *i1, Instruction *i2){
   
   if(i1->is_dep_RAW1(i2)) 
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": RAW1"<<endl; 
   if(i1->is_dep_RAW2(i2)) 
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": RAW2"<<endl;
   
   if(i1->is_dep_WAR(i2)) 
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": WAR"<<endl;
   
   if(i1->is_dep_WAW(i2)) 
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": WAW"<<endl;
   
   if(i1->is_dep_MEM(i2)) 
      cout<<"Dependance i"<<i1->get_index()<<"->i"<<i2->get_index()<<": MEM"<<endl;
   
}

Basic_block::Basic_block(){
   _head = NULL;
   _end = NULL;
   _branch = NULL;
   _index = 0;
   _nb_instr = 0;
   _firstInst=NULL;
   _lastInst=NULL;
   dep_done = false;
   use_def_done = false;
   live_in_out_done = false;
   
   for(int i=0; i<NB_REG; i++){
      Use[i]= false;
      LiveIn[i] = false;
      LiveOut[i] = false;
      Def[i] = false;
      DefLiveOut[i] = -1;
   }

 for(int i=0; i<NB_MAX_BB; i++){
      Domin[i]= true;
 }

}


Basic_block::~Basic_block(){}


void Basic_block::set_index(int i){
   _index = i;
}

int Basic_block::get_index(){
   return _index;
}

void Basic_block::set_head(Line *head){
   _head = head;
}

void Basic_block::set_end(Line *end){
   _end = end;
}

Line* Basic_block::get_head(){
   return _head;
}

Line* Basic_block::get_end(){
   return _end;
}

void Basic_block::set_successor1(Basic_block *BB){
   _succ.push_front(BB);
}

Basic_block *Basic_block::get_successor1(){
   if (_succ.size()>0)
      return _succ.front();
   else 
      return NULL;
}

void Basic_block::set_successor2(Basic_block *BB){	
   _succ.push_back(BB);
}

Basic_block *Basic_block::get_successor2(){
   if (_succ.size()> 1)
      return _succ.back();
   else 
      return NULL;
}

void Basic_block::set_predecessor(Basic_block *BB){
   _pred.push_back(BB);
}

Basic_block *Basic_block::get_predecessor(int index){

   list<Basic_block*>::iterator it;
   it=_pred.begin();
   int size=(int)_pred.size();
   if(index< size){
      for (int i=0; i<index; i++, it++);
      return *it;	
   }
   else cout<<"Error: index is bigger than the size of the list"<<endl; 	
   return _pred.back();
	
}

int Basic_block::get_nb_succ(){
   return _succ.size();
}

int Basic_block::get_nb_pred(){
   return _pred.size();
}

void Basic_block::set_branch(Line* br){
   _branch=br;
}

Line* Basic_block::get_branch(){
   return _branch;
}

void Basic_block::display(){
   cout<<"Begin BB"<<endl;
   Line* element = _head;
   int i=0;
   if(element == _end)	
      cout << _head->get_content() <<endl;
  
   while(element != _end->get_next()){
      if(element->isInst()){
	 cout<<"i"<<i<<" ";
	 i++;
      }
      if(!element->isDirective())
	 cout <<element->get_content() <<endl;
      
      element = element->get_next();
   }
   cout<<"End BB"<<endl;
}

int Basic_block::size(){
   Line* element = _head;
   int lenght=0;
   while(element != _end){
      lenght++;
      if(element->get_next()==_end)	
	 break;
      else 
	 element = element->get_next();
   }
   return lenght;
}	


void Basic_block::restitution(string const filename){	
   Line* element = _head;
   ofstream monflux(filename.c_str(), ios::app);
   if(monflux){
      monflux<<"Begin BB"<<endl;
      if(element == _end)	
	monflux << _head->get_content() <<endl;
      while(element != _end)
      {
	 if(element->isInst()) 
	    monflux<<"\t";
	 if(!element->isDirective())
	    monflux << element->get_content()<<endl ;
		
	 if(element->get_next()==_end){
	    if(element->get_next()->isInst()) 
	       monflux<<"\t";
	    if(!element->isDirective())
	       monflux << element->get_next()->get_content()<<endl;
	    break;
	 }
	 else element = element->get_next();
      }
      monflux<<"End BB\n\n"<<endl;		
   }
   else {
      cout<<"Error cannot open the file"<<endl;
   }
   monflux.close();

}

bool Basic_block::is_labeled(){
   if (_head->isLabel()){
      return true;
   }
   else return false;
}

int Basic_block::get_nb_inst(){   
   if (_nb_instr == 0)
      link_instructions();
   return _nb_instr;
    
}

Instruction* Basic_block::get_instruction_at_index(int index){
   Instruction *inst;
   
   if(index >= get_nb_inst()){
      return NULL;
   }
   
   inst=get_first_instruction();

   for(int i=0; i<index; i++, inst=inst->get_next());

   return inst;
}

Line* Basic_block::get_first_line_instruction(){
   
   Line *current = _head;
   while(!current->isInst()){
      current=current->get_next();
      if(current==_end)
	 return NULL;
   }
   return current;
}

Instruction* Basic_block::get_first_instruction(){
  if(_firstInst==NULL){
      _firstInst= getInst(this->get_first_line_instruction());
      this->link_instructions();
  }
   return _firstInst;
}

Instruction* Basic_block::get_last_instruction(){
   if(_lastInst==NULL)
      this->link_instructions();
   return _lastInst;
}


/* link_instructions  num�rote les instructions du bloc */
/* remplit le champ nb d'instructions du bloc (_nb_instr) */
/* remplit le champ derniere instruction du bloc (_lastInst) */
void Basic_block::link_instructions(){

   int index=0;
   Line *current, *next;
   current=get_first_line_instruction();
   next=current->get_next();

   Instruction *i1 = getInst(current);

   i1->set_index(index);
   index++;
   Instruction *i2;
   
//Calcul des successeurs
   while(current != _end){
   
      while(!next->isInst()){
	 next=next->get_next();
	 if(next==_end){
	    if(next->isInst())
	       break;
	    else{
	       _lastInst = i1;
	       _nb_instr = index;
	       return;
	    }
	 }
      }
      
      i2 = getInst(next);
      i2->set_index(index);
      index++;
      i1->set_link_succ_pred(i2);
      
      i1=i2;
      current=next;
      next=next->get_next();
   }
   _lastInst = i1;
   _nb_instr = index;
}

bool Basic_block::is_delayed_slot(Instruction *i){
   if (get_branch()== NULL)
      return false;
   int j = (getInst(get_branch()))->get_index();
   return (j < i-> get_index());

}

/* set_link_succ_pred : ajoute succ comme successeur de this et ajoute this comme pr�d�cesseur de succ
 */

void Basic_block::set_link_succ_pred(Basic_block* succ){
  _succ.push_back(succ);
  succ->set_predecessor(this);
}

/* add_dep_link ajoute la d�pendance avec pred � la liste des dependances pr�c�desseurs de succ */
/* ajoute la dependance avec succ � la liste des d�pendances successeurs de pred */

/* dep est une structure de donn�es contenant une instruction et  un type de d�pendance */

void add_dep_link(Instruction *pred, Instruction* succ, t_Dep type){
   dep *d;
   d=(dep*)malloc(sizeof(dep));
   d->inst=succ;
   d->type=type;
   pred->add_succ_dep(d);
   
   d=(dep*)malloc(sizeof(dep));
   d->inst=pred;
   d->type=type;
   succ->add_pred_dep(d);
}


/* calcul des d�pendances entre les instructions dans le bloc  */
/* une instruction a au plus 1 reg dest et 2 reg sources */
/* Attention le reg source peut �tre 2 fois le m�me */ 
/* Utiliser les m�thodes is_dep_RAW1, is_dep_RAW2, is_dep_WAR, i
 * s_dep_WAW, is_dep_MEM pour d�terminer les d�pendances */

/* ne pas oublier les d�pendances de controle avec le branchement s'il y en a un */

/* utiliser la fonction add_dep_link ci-dessus qui ajoute � la liste des d�pendances pred et succ 
 * une dependance entre 2 instructions */

void Basic_block::comput_pred_succ_dep(){
   
  // IMPORTANT : laisser les 2 instructions ci-dessous 
   link_instructions();
   if (dep_done) return;

   Line *current , *last;
   current = get_first_line_instruction();
   list<Line*>::iterator it;
   list<Line*> previous;
   
   while(current != _end)
   {
       if(current->isInst())
       {
            Instruction* tempCurrentInst = getInst(current);
                
            //WAR, WAW
            for(list<Line*>::reverse_iterator it = previous.rbegin() ; it != previous.rend() ; ++it)
            {
                Line* tempLine = *it;
                Instruction* tempPrevInstr = getInst(tempLine);
                if(tempPrevInstr->is_dep_WAW(tempCurrentInst))
                {
                    add_dep_link(tempPrevInstr, tempCurrentInst, WAW);
                    break;
                }
                
                if(tempPrevInstr->is_dep_WAR(tempCurrentInst))
                {
                    add_dep_link(tempPrevInstr, tempCurrentInst, WAR);
                    break;
                }
            }
            
            //RAW1
            for(list<Line*>::reverse_iterator it = previous.rbegin() ; it != previous.rend() ; ++it)
            {
                Line* tempLine = *it;
                Instruction* tempPrevInstr = getInst(tempLine);
                if(tempPrevInstr->is_dep_RAW1(tempCurrentInst))
                {
                    add_dep_link(tempPrevInstr, tempCurrentInst, RAW);
                    break;
                }
            }
            
            OPRegister* reg1 = tempCurrentInst->get_reg_src1();
            OPRegister* reg2 = tempCurrentInst->get_reg_src2();
            if(reg1 && reg2)
            {
                //RAW2
                for(list<Line*>::reverse_iterator it = previous.rbegin() ; it != previous.rend() ; ++it)
                {
                    Line* tempLine = *it;
                    Instruction* tempPrevInstr = getInst(tempLine);
                    if(tempPrevInstr->is_dep_RAW2(tempCurrentInst))
                    {
                        add_dep_link(tempPrevInstr, tempCurrentInst, RAW);
                        break;
                    }
                }
            }
            
            for(list<Line*>::reverse_iterator it = previous.rbegin() ; it != previous.rend() ; ++it)
            {
                Line* tempLine = *it;
                Instruction* tempPrevInstr = getInst(tempLine);
                if(tempPrevInstr->is_dep_MEM(tempCurrentInst))
                {
                    add_dep_link(tempPrevInstr, tempCurrentInst, MEMDEP);
                }
            }
            previous.push_back(current);
       }
       current = current->get_next();
   }

   last = previous.back();
   Instruction* lastInstr = getInst(last);
   
   if(lastInstr->is_branch())
   {
       for(list<Line*>::iterator it = previous.begin() ; it != previous.end() ; ++it)
       {
            Line* tempLine  = *it;
            Instruction* tempInstr = getInst(tempLine);
            if(lastInstr == tempInstr)
            {
                continue;
            }
            if(!tempInstr->get_nb_succ())
            {
                add_dep_link(tempInstr, lastInstr, CONTROL);
            }
       }
   }
   
   
   
   
   // NE PAS ENLEVER : cette fonction ne doit �tre appel�e qu'une seule fois
   dep_done = true;
   return;
}



void Basic_block::reset_pred_succ_dep(){

  Instruction *ic=get_first_instruction();
  while (ic){
    ic -> reset_pred_succ_dep();
    ic = ic -> get_next();
  }
  dep_done = false;
  return;
}


/* calcul le nb de cycles pour executer le BB, 
 * on suppose qu'une instruction peut sortir du pipeline � chaque cycle, 
 * il faut trouver les cycles de gel induit par les d�pendances */

int Basic_block::nb_cycles(){
  
    int nb_gels = 0;

    Line* current;
    current = get_first_line_instruction();
    
    while(current != _end)
    {
        if(current->isInst())
        {
            
            Instruction* currentInstr = getInst(current);
            int max_gel = 0;      
            for(int i = 0 ; i < currentInstr->get_nb_pred() ; i++)
            {
                
                dep* tempPred = currentInstr->get_pred_dep(i);
                if(tempPred->type == RAW)
                {
                    Instruction* tempPredInstr = tempPred->inst;
                    
                    int delaiBetween = delai(tempPredInstr->get_type(), current->get_type());
                    int nbInstrBetween = currentInstr->get_index() - tempPredInstr->get_index() - 1;
                    int temp_nb_gels = delaiBetween - 1 - nbInstrBetween ;
                    if(temp_nb_gels > 0)
                    {
                        if(temp_nb_gels > max_gel)
                        {
                            max_gel = temp_nb_gels;
                        }
                    }
                }
            }
            nb_gels += max_gel;
        }
        current = current->get_next();
    }
    
    return get_nb_inst() + nb_gels;
}

/* 
calcule DEF et USE pour l'analyse de registre vivant 
� la fin on doit avoir
 USE[i] vaut 1 si $i est utilis� dans le bloc avant d'�tre potentiellement d�fini dans le bloc
 DEF[i] vaut 1 si $i est d�fini dans le bloc 
******************/

void Basic_block::compute_use_def(void){

    if(use_def_done)
    {
        return;
    }

    Line* current;
    current = get_first_line_instruction();
    
    while(current != _end)
    {
        if(current->isInst())
        {
            
            Instruction* currentInstr = getInst(current);
            
            OPRegister* regSrc1 = currentInstr->get_reg_src1();
            OPRegister* regSrc2 = currentInstr->get_reg_src2();
            
            OPRegister* regDst = currentInstr->get_reg_dst();
            
            if(regSrc1)
            {
                int regNum = regSrc1->get_reg();
                if(!Def[regNum])
                {
                    Use[regNum] = true;
                }
            }
            if(regSrc2)
            {
                int regNum = regSrc2->get_reg();
                if(!Def[regNum])
                {
                    Use[regNum] = true;
                }
            }
            if(regDst)
            {
                int regNum = regDst->get_reg();
                Def[regNum] = true;
            }
            if(currentInstr->is_call()){
                Def[2] = true;
                if(!Def[4])
                {
                   Use[4] = true; 
                }
                if(!Def[5])
                {
                   Use[5] = true; 
                }
                if(!Def[6])
                {
                   Use[6] = true; 
                }
            }
            if(currentInstr->is_indirect_branch())
            {
                Def[2] = true;
                LiveOut[2] = true;
            }
        }
        current = current->get_next();
    }
    
    use_def_done = true;
    
    return;
}

/**** compute_def_liveout 
� la fin de la fonction on doit avoir
DefLiveOut[i] vaut l'index de l'instruction du bloc qui d�finit $i si $i vivant en sortie seulement
Si $i est d�fini plusieurs fois c'est l'instruction avec l'index le plus grand
*****/
void Basic_block::compute_def_liveout(){
    Line* current;
    current = get_first_line_instruction();
    
    while(current != _end)
    {
        if(current->isInst())
        {
            
            Instruction* currentInstr = getInst(current);
            OPRegister* regDst = currentInstr->get_reg_dst();
            
            if(regDst)
            {
                int regNum = regDst->get_reg();
                if(LiveOut[regNum])
                {
                    DefLiveOut[regNum] = currentInstr->get_index();
                }
            }
        }
        current = current->get_next();
    }
}



/**** renomme les registres renommables en utilisant comme registres disponibles 
 * ceux dont le num�ro est dans la liste param�tre 
*****/
void Basic_block::reg_rename(list<int> *frees){
 
    
    list<int> RegUsed;
    vector<int> DefUses[NB_REG];
    
    
    for(int i = 0 ; i < NB_REG ; i++)
    {
      if(Use[i] || Def[i])
      {
          RegUsed.push_back(i);
      }
    }
    
    for(list<int>::iterator it = RegUsed.begin() ; it != RegUsed.end() ; ++it)
    {
        bool defined = false;
        int numReg = *it;
        Line* current = get_first_line_instruction();
        while(current != _end)
        {
            if(current->isInst())
            {
                Instruction* currentInstr = getInst(current);
                OPRegister* regSrc1 = currentInstr->get_reg_src1();
                OPRegister* regSrc2 = currentInstr->get_reg_src2();

                OPRegister* regDst = currentInstr->get_reg_dst();

                if(regSrc1)
                {
                    if(defined)
                    {
                        if(regSrc1->get_reg() == numReg)
                        {
                            DefUses[numReg].push_back(currentInstr->get_index());
                            defined = true;
                        }
                    }
                }
                if(regSrc2)
                {
                    if(defined)
                    {
                        if(regSrc2->get_reg() == numReg)
                        {
                            DefUses[numReg].push_back(currentInstr->get_index());
                            defined = true;
                        }
                    }
                }
                if(regDst)
                {
                    if(regDst->get_reg() == numReg)
                    {
                        if(!defined)
                        {
                            if(currentInstr->get_index() != DefLiveOut[numReg])
                            {
                                DefUses[numReg].push_back(currentInstr->get_index());
                                defined = true;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            current = current->get_next();
        }
    }
    
    for(int i = 0 ; i < NB_REG ; i++)
    {
        if(frees->empty())
        {
            break;
        }
        
        if(!DefUses[i].empty())
        {
            int newNumReg = frees->front();
            frees->pop_front();
            for(int j = 0 ; j < DefUses[i].size() ; j++)
            {
                Instruction* instr = get_instruction_at_index(j);
                OPRegister* regSrc1 = instr->get_reg_src1();
                OPRegister* regSrc2 = instr->get_reg_src2();

                OPRegister* regDst = instr->get_reg_dst();

                if(regSrc1)
                {
                    if(regSrc1->get_reg() == i)
                    {
                        regSrc1->set_reg(newNumReg);
                    }
                }
                if(regSrc2)
                {
                    if(regSrc2->get_reg() == i)
                    {
                        regSrc1->set_reg(newNumReg);
                    }
                }
                if(regDst)
                {
                    if(regDst->get_reg() == i)
                    {
                        regSrc1->set_reg(newNumReg);
                    }
                }
            }
        }
    }
}


/**** renomme les registres renommables en utilisant comme registres disponibles 
 * ceux disponible pour le bloc d'apr�s l'analyse de vivacit� et def-use

*****/
void Basic_block::reg_rename(){

  list<int> frees;
  
  for(int i = 0 ; i < NB_REG ; i++)
  {
      if(!(Use[i] || Def[i] || LiveIn[i] || LiveOut[i]))
      {
          frees.push_back(i);
      }
  }
  
  reg_rename(&frees);
}


void Basic_block::apply_scheduling(list <Node_dfg*> *new_order){
   list <Node_dfg*>::iterator it=new_order->begin();
   Instruction *inst=(*it)->get_instruction();
   Line *n=_head, *prevn=NULL;
   Line *end_next = _end->get_next();
   if(!n){
      cout<<"wrong bb : cannot apply"<<endl;
      return;
   }
   
   while(!n->isInst()){
     prevn=n;
     n=n->get_next();
     if(n==_end){
       cout<<"wrong bb : cannot apply"<<endl;
       return;
     }
   }
   
   //y'a des instructions, on sait pas si c'est le bon BB, mais on va supposer que oui
   inst->set_index(0);
   inst->set_prev(NULL);
   _firstInst = inst;
   n = inst;
   
   if(prevn){
     prevn->set_next(n);
     n->set_prev(prevn);
   }
   else{
     set_head(n);
   }

   int i;
   it++;
   for(i=1; it!=new_order->end(); it++, i++){

     inst->set_link_succ_pred((*it)->get_instruction());
     
     inst=(*it)->get_instruction();
     inst->set_index(i);
     prevn = n;
     n = inst;
     prevn->set_next(n);
     n->set_prev(prevn);
     }
   inst->set_next(NULL);
   _lastInst = inst;
   set_end(n);
   n->set_next(end_next);
   return;
}

/* permet de tester des choses sur un bloc de base, par exemple la construction d'un DFG, � venir ... l� ne fait rien qu'afficher le BB */
void Basic_block::test(){
   cout << "test du BB " << get_index() << endl;
   display();
   cout << "nb de successeur : " << get_nb_succ() << endl;
   int nbsucc = get_nb_succ() ;
   if (nbsucc >= 1 && get_successor1())
      cout << "succ1 : " << get_successor1()-> get_index();
   if (nbsucc >= 2 && get_successor2())
      cout << " succ2 : " << get_successor2()-> get_index();
   cout << endl << "nb de predecesseurs : " << get_nb_pred() << endl ;
  
   int size=(int)_pred.size();
   for (int i = 0; i < size; i++){
      if (get_predecessor(i) != NULL)
	 cout << "pred "<< i <<  " : " << get_predecessor(i)-> get_index() << "; ";
   }
   cout << endl;
   cout << "Dominants : { ";
   for(int i = 0 ; i < NB_MAX_BB ; i++)
   {
       if(Domin[i])
       {
           cout << i << ", ";
       }
   }
   cout << "}" << endl << endl;
   
   cout << "Use du BB : { ";
   for(int i = 0 ; i < NB_REG ; i++)
   {
       if(Use[i])
       {
           cout << i << ", ";
       }
   }
   cout << "}" << endl << endl;
   
   cout << "Def du BB : { ";
   for(int i = 0 ; i < NB_REG ; i++)
   {
       if(Def[i])
       {
           cout << i << ", ";
       }
   }
   cout << "}" << endl << endl;
   
   cout << "LiveIn du BB : { ";
   for(int i = 0 ; i < NB_REG ; i++)
   {
       if(LiveIn[i])
       {
           cout << i << ", ";
       }
   }
   cout << "}" << endl << endl;
   
   cout << "LiveOut du BB : { ";
   for(int i = 0 ; i < NB_REG ; i++)
   {
       if(LiveOut[i])
       {
           cout << i << ", ";
       }
   }
   cout << "}" << endl << endl;
   
   cout << "DefLiveOut du BB : { " << endl;
   for(int i = 0 ; i < NB_REG ; i++)
   {
       if(DefLiveOut[i] != -1)
       {
           cout << "Reg num : " << i << " , Index instr : "<< DefLiveOut[i] << endl ;
       }
   }
   cout << "}" << endl << endl;
}
