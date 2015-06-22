#include <Function.h>

Function::Function() {
    _head = NULL;
    _end = NULL;
    BB_computed = false;
    BB_pred_succ = false;
    dom_computed = false;
}

Function::~Function() {
}

void Function::set_head(Line *head) {
    _head = head;
}

void Function::set_end(Line *end) {
    _end = end;
}

Line* Function::get_head() {
    return _head;
}

Basic_block* Function::get_firstBB() {
    return _myBB.front();
}

Line* Function::get_end() {
    return _end;
}

void Function::display() {
    cout << "Begin Function" << endl;
    Line* element = _head;

    if (element == _end)
        cout << _head->get_content() << endl;

    while (element != _end) {
        cout << element->get_content() << endl;

        if (element->get_next() == _end) {
            cout << element->get_next()->get_content() << endl;
            break;
        } else element = element->get_next();

    }
    cout << "End Function\n\n" << endl;

}

int Function::size() {
    Line* element = _head;
    int lenght = 0;
    while (element != _end) {
        lenght++;
        if (element->get_next() == _end)
            break;
        else
            element = element->get_next();
    }
    return lenght;
}

void Function::restitution(string const filename) {

    Line* element = _head;
    ofstream monflux(filename.c_str(), ios::app);

    if (monflux) {
        monflux << "Begin" << endl;
        if (element == _end)
            monflux << _head->get_content() << endl;
        while (element != _end) {
            if (element->isInst() ||
                    element->isDirective())
                monflux << "\t";

            monflux << element->get_content();

            if (element->get_content().compare("nop"))
                monflux << endl;

            if (element->get_next() == _end) {
                if (element->get_next()->isInst() ||
                        element->get_next()->isDirective())
                    monflux << "\t";
                monflux << element->get_next()->get_content() << endl;
                break;
            } else element = element->get_next();

        }
        monflux << "End\n\n" << endl;

    }
    else {
        cout << "Error cannot open the file" << endl;
    }

    monflux.close();
}

void Function::comput_label() {
    Line* element = _head;

    if (element == _end && element->isLabel())
        _list_lab.push_back(getLabel(element));
    while (element != _end) {

        if (element->isLabel())
            _list_lab.push_back(getLabel(element));

        if (element->get_next() == _end) {
            if (element->isLabel())
                _list_lab.push_back(getLabel(element));
            break;
        } else element = element->get_next();

    }

}

int Function::nbr_label() {
    return _list_lab.size();

}

Label* Function::get_label(int index) {

    list<Label*>::iterator it;
    it = _list_lab.begin();

    int size = (int) _list_lab.size();
    if (index < size) {
        for (int i = 0; i < index; i++) it++;
        return *it;
    } else cout << "Error get_label : index is bigger than the size of the list" << endl;

    return _list_lab.back();
}

Basic_block *Function::find_label_BB(OPLabel* label) {
    //Basic_block *BB = new Basic_block();
    int size = (int) _myBB.size();
    string str;
    for (int i = 0; i < size; i++) {
        if (get_BB(i)->is_labeled()) {

            str = get_BB(i)->get_head()->get_content();
            if (!str.compare(0, (str.size() - 1), label->get_op())) {
                return get_BB(i);
            }
        }
    }
    return NULL;
}

/* ajoute nouveau BB � la liste de BB de la fonction en le creant */

void Function::add_BB(Line *debut, Line* fin, Line *br, int index) {
    Basic_block *b = new Basic_block();
    b->set_head(debut);
    b->set_end(fin);
    b->set_index(index);
    b->set_branch(br);
    _myBB.push_back(b);
}


//Calcule la liste des blocs de base : il faut d�limiter les BB, en parcourant 
//la liste des lignes/instructions � partir de la premiere, il faut s'arreter 
// chaque branchement (et prendre en compte le delayed slot qui appartient au meme BB,
//c'est l'instruction qui suit tout branchement) ou � chaque label 
//(on estime que tout label est utilis� par un saut et donc correspond bien � une entete de BB).

void Function::comput_basic_block() {
    cout<< "debut comput Basick block" << endl;
    Line *debut, *current, *prev, *end;
    current = _head;
    debut = _head;
    end = _end;
    prev = NULL;
    int ind = 0;
    Line *l = NULL;
    Instruction *i = NULL;
    Line * b;

    //ne pas enlever la ligne ci-dessous 
    if (BB_computed) return;

    /**** A COMPLETER ****/
    
    while(current->isDirective())
    {
        current = current->get_next();
    }
    debut = current;
    current = current->get_next();
    
    while(current != _end)
    {
        if(current->isLabel())
        {
            end = current->get_prev();
            add_BB(debut, end, NULL, ind++);
            debut = current;
        }
        if(current->isInst())
        {
            i = getInst(current);
            if(i->is_branch())
            {
                end = current->get_next();
                add_BB(debut, end, current, ind++);
                debut = end->get_next();
                if(debut->isLabel())
                {
                    current = debut;
                }
            }
        }
        current = current->get_next();
    }
    
    BB_computed = true;
    return;
}

int Function::nbr_BB() {
    return _myBB.size();
}

Basic_block *Function::get_BB(int index) {

    list<Basic_block*>::iterator it;
    it = _myBB.begin();
    int size = (int) _myBB.size();

    if (index < size) {
        for (int i = 0; i < index; i++) it++;
        return *it;
    } else
        return NULL;
}

list<Basic_block*>::iterator Function::bb_list_begin() {
    return _myBB.begin();
}

list<Basic_block*>::iterator Function::bb_list_end() {
    return _myBB.end();
}

/* comput_pred_succ calcule les successeurs et pr�d�cesseur des BB, 
 * pour cela il faut commencer par les successeurs */
/* et it�rer sur tous les BB d'une fonction */
/* il faut determiner si un BB a un ou deux successeurs : d�pend de la pr�sence d'un saut pr�sent ou non � la fin */
/* pas de saut ou saut incontionnel ou appel de fonction : 1 successeur (lequel ?)*/
/* branchement conditionnel : 2 successeurs */
/* le dernier bloc n'a pas de successeurs - celui qui se termine par jr R31 */

/* les sauts indirects n'ont pas de successeur */


void Function::comput_succ_pred_BB() {

    list<Basic_block*>::iterator it;
    Basic_block *current;
    Instruction *instr;
    Basic_block *succ = NULL;
    // IMPORTANT ne pas enlever la ligne ci-dessous 
    if (BB_pred_succ) return;
    int size = (int) _myBB.size();
    it = _myBB.begin();

    for (int i = 0; i < size; i++) {
        current = *it;

        Line* current_line = current->get_branch();
        if(current_line)
        {
            if(instr = getInst(current_line))
            {
                if(instr->is_call())
                {
                    if(succ = get_BB(i+1))
                    {
                        current->set_link_succ_pred(succ);
                    }
                }
                else if(instr->is_cond_branch())
                {
                    if(succ = get_BB(i+1))
                    {
                        current->set_link_succ_pred(succ);
                    }
                    if(succ = find_label_BB(instr->get_op_label()))
                    {
                        current->set_link_succ_pred(succ);
                    }
                    
                }
                else if(!instr->is_indirect_branch())
                {
                    if(succ = find_label_BB(instr->get_op_label()))
                    {
                        current->set_link_succ_pred(succ);
                    }
                }
            }
        }
        else
        {
            if(succ = get_BB(i+1))
            {
                current->set_link_succ_pred(succ);
            }
        }

        it++;
    }

    // ne pas enlever la ligne ci-dessous
    BB_pred_succ = true;
    return;
}

void Function::compute_dom() {
    list<Basic_block*>::iterator it, it2;
    list<Basic_block*> workinglist;
    Basic_block *current, *bb, *pred;
    Instruction *instr;
    bool change = false;

    // NE pas enlever les 2 ligne ci-dessous
    if (dom_computed) return;
    comput_succ_pred_BB();


    // A COMPLETER 
    current = get_BB(0);
    workinglist.push_back(current);
    for(int i = 0 ; i < NB_MAX_BB ; i++)
    {
        current->Domin[i] = false;
    }
    current->Domin[current->get_index()] = true;
    bb = current->get_successor1();
    if(bb)
    {
        workinglist.push_back(bb);
    }
    bb = current->get_successor2();
    if(bb)
    {
        workinglist.push_back(bb);
    }
    
    while(!workinglist.empty())
    {
        current = workinglist.front();
        workinglist.pop_front();
        for(int i = 0 ; i < current->get_nb_pred() ; i++)
        {
            bb = current->get_predecessor(i);
            for(int j = 0 ; j < NB_MAX_BB ; j++)
            {
                if(j == current->get_index())
                {
                    continue;
                }
                bool before = current->Domin[j];
                current->Domin[j] = current->Domin[j] && bb->Domin[j];
                if(before != current->Domin[j])
                {
                    change = true;
                }
            }
        }
        
        if(change)
        {
            bb = current->get_successor1();
            if(bb)
            {
                workinglist.push_back(bb);
            }
            bb = current->get_successor2();
            if(bb)
            {
                workinglist.push_back(bb);
            }
        }
        change = false;
    }
    
    
    // ne pas enlever 
    dom_computed = true;
    return;
}

void Function::compute_live_var() {

    list<Basic_block*>::iterator it;
    
    list<Basic_block*> blockToDo;
    
    int nbDone = 0;

    Basic_block* current = NULL;
    
    bool can_compute_in = true;
    
    int size = (int) _myBB.size();
    it = _myBB.begin();

    for (int i = 0; i < size; i++) {
        current = *it;
        
        current->compute_use_def();
        if(!current->get_nb_succ())
        {
            blockToDo.push_back(current);
        }

        it++;
    }
    
    while(nbDone != nbr_BB())
    {
        current = blockToDo.front();
        if(!current)
        {
            break;
        }
        if(!current->live_in_out_done)
        {
            can_compute_in = true;
            if(current->get_nb_succ() == 1)
            {
                Basic_block* succ1 = current->get_successor1();
                if(succ1->live_in_out_done)
                {
                    for(int i = 0 ; i < NB_REG ; i++)
                    {
                        current->LiveOut[i] = succ1->LiveIn[i];
                    }
                }
                else
                {
                    can_compute_in = false;
                }
            }
            if(current->get_nb_succ() == 2)
            {
                Basic_block* succ1 = current->get_successor1();
                Basic_block* succ2 = current->get_successor2();
                if(succ1->live_in_out_done && succ2->live_in_out_done)
                {
                    for(int i = 0 ; i < NB_REG ; i++)
                    {
                        current->LiveOut[i] =  succ1->LiveIn[i]
                                                || succ2->LiveIn[i];
                    }
                }
                else
                {
                    can_compute_in = false;
                }
            }
            if(can_compute_in)
            {
                bool out_without_def[NB_REG];
                for(int i = 0 ; i < NB_REG ; i++)
                {
                    if(current->LiveOut[i] && !current->Def[i])
                    {
                        out_without_def[i] = true;
                    }
                    else
                    {
                        out_without_def[i] = false;
                    }
                    current->LiveIn[i] = current->Use[i] || out_without_def[i];
                }
                current->live_in_out_done = true;
            }
            else
            {
                blockToDo.pop_front();
            }
            if(current->live_in_out_done)
            {
                nbDone++;
                blockToDo.pop_front();
                for(int i = 0 ; i < current->get_nb_pred() ; i++)
                {
                    Basic_block* tempPred = current->get_predecessor(i);
                    if(!tempPred->live_in_out_done)
                    {
                        blockToDo.push_back(tempPred);
                    }
                }
            }
        }
        else
        {
            blockToDo.pop_front();
        }
    }
    
    return;
}

/* en implementant la fonction test de la classe BB, permet de tester des choses sur tous les blocs de base d'une fonction par exemple l'affichage de tous les BB d'une fonction ou l'affichage des succ/pred des BBs comme c'est le cas -- voir la classe Basic_block et la m�thode test */

void Function::test() {
    int size = (int) _myBB.size();
    for (int i = 0; i < size; i++) {
        get_BB(i)->test();
    }
    return;
}
