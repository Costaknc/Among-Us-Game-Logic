/***************************************************
 *                                                 *
 * file: among_us.c                                *
 *                                                 *
 * @brief   Implementation of among_us.h           *
 *                                                 *
 ***************************************************
 */

#include "among_us.h"

/**
 * @brief Optional function to initialize data structures that 
 *        need initialization
 *
 * @return 1 on success
 *         0 on failure
 */
int initialize() {

	players_head = (struct Players*)malloc(sizeof(struct Players));
	players_head->prev = players_head;
	players_head->next = players_head;

	tasks_head = (struct Head_GL*)malloc(sizeof(struct Head_GL));
    tasks_head->head = NULL;
    tasks_head->tasks_count[0] = 0;
    tasks_head->tasks_count[1] = 0;
    tasks_head->tasks_count[2] = 0;

    tasks_stack = (struct Head_Completed_Task_Stack*)malloc(sizeof(struct Head_Completed_Task_Stack));
    tasks_stack->head = (struct Tasks*)malloc(sizeof(struct Tasks));
    tasks_stack->head = NULL;
    tasks_stack->count = 0;

    return 1;
}

/**
 * @brief Register player
 *
 * @param pid The player's id
 *
 * @param is_alien The variable that decides if he is an alien or not
 * @return 1 on success
 *         0 on failure
 */
int register_player(int pid,int is_alien) {
    struct Players *P_new = NULL;
    struct Players *tmp;
    P_new = (struct Players*)malloc(sizeof(struct Players));

    P_new->pid = pid;
    P_new->is_alien = is_alien;
    P_new->evidence = 0;

    P_new->next = players_head->next;
    P_new->prev = players_head;
    players_head->next = P_new;
    P_new->next->prev = P_new;
    P_new->tasks_sentinel = (struct Tasks*)malloc(sizeof(struct Tasks));
    P_new->tasks_head = P_new->tasks_sentinel;

    tmp = players_head->next;

    print_players();

    return 1;
}

/**
 * @brief Insert task in the general task list
 *
 * @param tid The task id
 * 
 * @param difficulty The difficulty of the task
 *
 * @return 1 on success
 *         0 on failure
 */
int insert_task(int tid,int difficulty) {

	struct Tasks *T_new = NULL;
	T_new = (struct Tasks*)malloc(sizeof(struct Tasks));

	struct Tasks *curr;
    struct Tasks *prev;

    struct Tasks *tmp;

	T_new->tid = tid;
	T_new->difficulty = difficulty;
	tasks_head->tasks_count[difficulty - 1]++;

	if(tasks_head->head == NULL){
		tasks_head->head = T_new;
        T_new->next = NULL;
	}
	else{
        prev = tasks_head->head;
        curr = prev->next;

        if(prev->difficulty == T_new->difficulty){
            tasks_head->head = T_new;
            T_new->next = prev;
        }
        else{
        	while((curr != NULL) && (curr->difficulty < T_new->difficulty)){
                prev = curr;
        		curr = curr->next;
            }
            prev->next = T_new;
            T_new->next = curr;
        }
	}

    print_tasks();

    return 1;
}

/**
 * @brief Distribute tasks to the players
 * @return 1 on success
 *         0 on failure
 */
int distribute_tasks() {

	struct Players *currP = players_head->next;
	struct Players *tmpP = players_head->next;
	struct Tasks *tmpT = tasks_head->head;
	struct Tasks *currT;

//traverse the tasks list for task assignment
	while(tmpT != NULL){

		currT = (struct Tasks*)malloc(sizeof(struct Tasks));
		currT->tid = tmpT->tid;
		currT->difficulty = tmpT->difficulty;

		if((currP->tasks_head == currP->tasks_sentinel) && (currP->is_alien == 0)){
			currP->tasks_head = currT;
			currP->tasks_sentinel->next = currT;
			currT->next = currP->tasks_sentinel;
		}
		else if((currP->tasks_head == currP->tasks_sentinel) && (currP->is_alien == 0)){
			currP->tasks_sentinel->next->next = currT;
			currT->next = currP->tasks_sentinel;
			currP->tasks_sentinel->next = currT;
		}
	    tmpT = tmpT->next;
	    currP = currP->next;

	    if(currP == players_head){
	    	currP = currP->next;
	    }
	}

	print_double_list();
    return 1;
}

/**
 * @brief Implement Task
 *
 * @param pid The player's id
 *
 * @param difficulty The task's difficulty
 *
 * @return 1 on success
 *         0 on failure
 */
int implement_task(int pid, int difficulty) {

	struct Players *currP = players_head->next;
	struct Tasks *currT;
	struct Tasks *delT;
	struct Tasks *tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));
	struct Players *newTmpP = NULL;
	struct Tasks *newTmpT = NULL;

	while(currP != players_head && currP->pid != pid){
		if(currP->pid == pid){
			break;
		}
		else if(currP == players_head->prev && currP->pid != pid){
			return 1;
		}
		else currP = currP->next;
	}

	currT = currP->tasks_head;

	while(currT != currP->tasks_sentinel && currT->difficulty != difficulty){
		currT = currT->next;
	}

	if(currT == currP->tasks_sentinel){
		currT = currP->tasks_sentinel->next;
	}

	tmpT->tid = currT->tid;
	tmpT->difficulty = currT->difficulty;

	if(currP->pid == pid && currT != currP->tasks_sentinel){
		tmpT->next = tasks_stack->head;
		tasks_stack->head = tmpT;

		tasks_head->tasks_count[tmpT->difficulty] = tasks_head->tasks_count[tmpT->difficulty]--;
		tasks_stack->count++;
	}

	delT = currP->tasks_head;

	if(delT == currP->tasks_sentinel->next){
		currP->tasks_head = currP->tasks_sentinel;
	}

	while(delT->next != currP->tasks_sentinel->next){
		delT = delT->next;
	}

	if(delT == currP->tasks_head){
		currP->tasks_head = currP->tasks_sentinel;
	}
	else{
		delT->next = currP->tasks_sentinel;
		currP->tasks_sentinel->next = delT;
	}

	print_double_list();
	print_stack();

    return 1;
}

/**
 * @brief Eject Player
 * 
 * @param pid The ejected player's id
 *
 * @return 1 on success
 *         0 on failure
 */
int eject_player(int pid) {

	struct Players *ejectedP = players_head->next;
	struct Players *prevP = NULL;
	struct Players *tmpP = players_head->next;
	struct Players *pid_min =NULL;
	struct Tasks *newT = NULL; //ejected players' task list
	struct Tasks *pid_min_T; //min tasks players' task list
	struct Tasks *tmpT = NULL;

	struct Tasks *prevT = NULL;

	int TaskCount = 0;
	int minTaskCount = 0;

	//find the player and delete him from Players List
	while(ejectedP != players_head && ejectedP->pid != pid){
		ejectedP = ejectedP->next;
	}

	prevP = ejectedP;
	prevP->prev->next = ejectedP->next;
	prevP->next->prev = ejectedP->prev;

	//find the player with the least tasks
	while(tmpP != players_head){

		while(tmpP->is_alien != 0){
			tmpP = tmpP->next;
		}

		tmpT = tmpP->tasks_head;

		while(tmpT != tmpP->tasks_sentinel){
			TaskCount++;
			tmpT = tmpT->next;
		}

		if(tmpP->tasks_head == tmpP->tasks_sentinel){
			pid_min = tmpP;

			break;
		}
		else if(TaskCount < minTaskCount){
			pid_min = tmpP;

			minTaskCount = TaskCount;
		}

		tmpP = tmpP->next;
	}

	newT = ejectedP->tasks_head;
	pid_min_T = pid_min->tasks_head;

	//merge the two players' tasks lists together
	if(pid_min->tasks_head == pid_min->tasks_sentinel){
		while(newT != ejectedP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			pid_min->tasks_head = tmpT;
			pid_min->tasks_sentinel->next = tmpT;
			tmpT->next = pid_min->tasks_sentinel;

			newT = newT->next;
		}
	}
	else if(pid_min_T->difficulty == tmpT->difficulty){
		while(newT != ejectedP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			tmpT->next = pid_min->tasks_head;
			pid_min->tasks_head = tmpT;

			newT = newT->next;
		}
	}
	else{
		while(pid_min_T != pid_min->tasks_sentinel && pid_min_T->difficulty < tmpT->difficulty){
			pid_min_T = pid_min_T->next;
		}
		while(newT != ejectedP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			if(pid_min_T->next == pid_min->tasks_sentinel){
				tmpT->next = pid_min->tasks_sentinel;
				pid_min->tasks_sentinel->next->next = tmpT;
				pid_min->tasks_sentinel->next = tmpT;
			}
			else{
				tmpT->next = pid_min_T->next;
				pid_min_T->next = tmpT;
			}

			newT = newT->next;
		}
	}

    print_double_list();

    return 1;
}

/**
 * @brief Witness Eject Player
 *
 * @param pid_a The alien's pid
 * 
 * @param pid The crewmate's pid
 * 
 * @param number_of_witnesses The number of witnesses
 *
 * @return 1 on success
 *         0 on failure
 */
int witness_eject(int pid, int pid_a, int number_of_witnesses){

	struct Players *alien = players_head->next;

	struct Players *ejectedP = players_head->next;
	struct Players *prevP = NULL;
	struct Players *tmpP = players_head->next;
	struct Players *pid_min =NULL;
	struct Tasks *newT = NULL; //ejected players' task list
	struct Tasks *pid_min_T = NULL; //min tasks players' task list
	struct Tasks *tmpT = NULL;
	struct Tasks *prevT = NULL;

	struct Players *newTmpP = NULL;
	struct Tasks *newTmpT = NULL;

	int TaskCount = 0;
	int minTaskCount = 0;

	//find the player and delete him from Players List
	while(ejectedP != players_head){
		if(ejectedP->pid == pid){
			break;
		}
		else if(ejectedP == players_head->prev && ejectedP->pid != pid){
			return 1;
		}
		else ejectedP = ejectedP->next;
	}

	prevP = ejectedP;
	prevP->prev->next = ejectedP->next;
	prevP->next->prev = ejectedP->prev;

	//find the player with the least tasks
	while(tmpP != players_head){

		while(tmpP->is_alien != 0){
			tmpP = tmpP->next;
		}

		tmpT = tmpP->tasks_head;

		while(tmpT != tmpP->tasks_sentinel){
			TaskCount++;
			tmpT = tmpT->next;
		}

		if(tmpP->tasks_head == tmpP->tasks_sentinel){
			pid_min = tmpP;

			// pid_min->pid = tmpP->pid;
			// pid_min->prev = tmpP->prev;
			// pid_min->next = tmpP->next;
			// pid_min->tasks_head = tmpP->tasks_head;
			// pid_min->tasks_sentinel = tmpP->tasks_sentinel;

			break;
		}
		else if(TaskCount < minTaskCount){
			pid_min = tmpP;

			minTaskCount = TaskCount;
		}

		tmpP = tmpP->next;
	}

	newT = ejectedP->tasks_head;
	pid_min_T = pid_min->tasks_head;

	//merge the two players' tasks lists together
	if(pid_min->tasks_head == pid_min->tasks_sentinel){
		while(newT != ejectedP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			pid_min->tasks_head = tmpT;
			pid_min->tasks_sentinel->next = tmpT;
			tmpT->next = pid_min->tasks_sentinel;

			newT = newT->next;
		}
	}
	else if(pid_min_T->difficulty == tmpT->difficulty){
		while(newT != ejectedP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			tmpT->next = pid_min->tasks_head;
			pid_min->tasks_head = tmpT;

			newT = newT->next;
		}
	}
	else{
		while(pid_min_T != pid_min->tasks_sentinel && pid_min_T->difficulty < tmpT->difficulty){
			pid_min_T = pid_min_T->next;
		}
		while(newT != ejectedP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			if(pid_min_T->next == pid_min->tasks_sentinel){
				tmpT->next = pid_min->tasks_sentinel;
				pid_min->tasks_sentinel->next->next = tmpT;
				pid_min->tasks_sentinel->next = tmpT;
			}
			else{
				tmpT->next = pid_min_T->next;
				pid_min_T->next = tmpT;
			}

			newT = newT->next;
		}
	}

	while(alien != players_head && alien->pid != pid_a){
		alien = alien->next;
	}

	//printing the "custom" message
	alien->evidence += number_of_witnesses;

	newTmpP = players_head->next;

    while(newTmpP != players_head){

        newTmpT = newTmpP->tasks_head;

        printf("\tPlayer <%d,%d> =", newTmpP->pid, newTmpP->evidence);

	   while(newTmpT != newTmpP->tasks_sentinel){
            printf("<%d,%d> ", newTmpT->tid, newTmpT->difficulty);
            newTmpT = newTmpT->next;
        }
        printf("\n");
        newTmpP = newTmpP->next;
    }
	printf("\nDONE\n\n");

    return 1;
}


/**
 * @brief Sabbotage
 *
 * @param pid The player's id
 *
 * @param number_of_tasks The number of tasks to be popped
 * 
 * @return 1 on success
 *         0 on failure
 */
int sabbotage(int pid, int number_of_tasks) {

	struct Tasks *CmplTasks = tasks_stack->head;
	struct Tasks *CmplTasksTmp = NULL;
	struct Tasks *tmpT = NULL;
	struct Tasks *currT = NULL;

	struct Players *currP = players_head->next;

	int x, y;

	//finding the player with <pid> = pid
		while(currP != players_head){
			if(currP->pid == pid){
				break;
			}
			else if(currP == players_head->prev && currP->pid != pid){
				return 1;
			}
			else currP = currP->next;
		}

		//for loop for going back in the players list
		for(y = 0; y < (int)(number_of_tasks/2); y++){
			currP = currP->prev;
		}


	//a for loop for every task we "pop"
	for(x = 0; x < (int)number_of_tasks; x++){

		if(CmplTasks == NULL){
			break;
		}

		while(currP->is_alien != 0){
			currP = currP->next;
		}
		if(currP == players_head){
				currP = currP->next;
		}

		currT = currP->tasks_head;

		//assigning the tasks to the players
		if(currP->tasks_head == currP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = CmplTasks->tid;
			tmpT->difficulty = CmplTasks->difficulty;

			currP->tasks_head = tmpT;
			currP->tasks_sentinel->next = tmpT;
			tmpT->next = currP->tasks_sentinel;

		}
		else{
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = CmplTasks->tid;
			tmpT->difficulty = CmplTasks->difficulty;

			while(currT != currP->tasks_sentinel && currT->difficulty < tmpT->difficulty){
				currT = currT->next;
			}

			if(currT->difficulty == tmpT->difficulty){
				tmpT->next = currP->tasks_head;
				currP->tasks_head = tmpT;
			}
			else if(currT->next == currP->tasks_sentinel){
				tmpT->next = currP->tasks_sentinel;
				currP->tasks_sentinel->next->next = tmpT;
				currP->tasks_sentinel->next = tmpT;
			}
			else{
				tmpT->next = currT->next;
				currT->next = tmpT;
			}
		}

		currP = currP->next;
		CmplTasks = CmplTasks->next;
		tasks_stack->head = CmplTasks;
		tasks_stack->count--;
	}

	print_double_list();

    return 1;
}


/**
 * @brief Vote
 *
 * @param pid The player's id
 * 
 * @param vote_evidence The vote's evidence
 *
 * @return 1 on success
 *         0 on failure
 */
int vote(int pid, int vote_evidence) {
	struct Players *currP = players_head->next;
	struct Players *mostSusP = NULL;

	struct Players *prevP = NULL;
	struct Players *tmpP = players_head->next;
	struct Players *pid_min =NULL;
	struct Tasks *newT = NULL; //ejected players' task list
	struct Tasks *pid_min_T; //min tasks players' task list
	struct Tasks *tmpT = NULL;

	struct Players *newTmpP = NULL;
	struct Tasks *newTmpT = NULL;

	struct Tasks *prevT = NULL;

	int TaskCount = 0;
	int minTaskCount = 0;

	int top = 0;

	while(currP != players_head && currP->pid != pid){

		if(currP->pid == pid){
			break;
		}
		else if(currP == players_head->prev && currP->pid != pid){
			return 1;
		}
		else currP = currP->next;
	}

	currP->evidence += vote_evidence;

	currP = players_head->next;

	while(currP != players_head){
		if(top < currP->evidence){
			top = currP->evidence;
			mostSusP = currP;
		}
		currP = currP->next;
	}

	prevP = mostSusP;
	prevP->prev->next = mostSusP->next;
	prevP->next->prev = mostSusP->prev;

	//find the player with the least tasks
	while(tmpP != players_head){

		tmpT = tmpP->tasks_head;

		while(tmpT != tmpP->tasks_sentinel){
			TaskCount++;
			tmpT = tmpT->next;
		}

		if(tmpP->tasks_head == tmpP->tasks_sentinel){
			pid_min = tmpP;

			break;
		}
		else if(TaskCount < minTaskCount){
			pid_min = tmpP;

			minTaskCount = TaskCount;
		}

		tmpP = tmpP->next;
	}

	newT = mostSusP->tasks_head;
	pid_min_T = pid_min->tasks_head;

	//merge the two players' tasks lists together
	if(pid_min->tasks_head == pid_min->tasks_sentinel){
		while(newT != mostSusP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			pid_min->tasks_head = tmpT;
			pid_min->tasks_sentinel->next = tmpT;
			tmpT->next = pid_min->tasks_sentinel;

			newT = newT->next;
		}
	}
	else if(pid_min_T->difficulty == tmpT->difficulty){
		while(newT != mostSusP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			tmpT->next = pid_min->tasks_head;
			pid_min->tasks_head = tmpT;

			newT = newT->next;
		}
	}
	else{
		while(pid_min_T != pid_min->tasks_sentinel && pid_min_T->difficulty < tmpT->difficulty){
			pid_min_T = pid_min_T->next;
		}
		while(newT != mostSusP->tasks_sentinel){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = newT->tid;
			tmpT->difficulty = newT->difficulty;

			if(pid_min_T->next == pid_min->tasks_sentinel){
				tmpT->next = pid_min->tasks_sentinel;
				pid_min->tasks_sentinel->next->next = tmpT;
				pid_min->tasks_sentinel->next = tmpT;
			}
			else{
				tmpT->next = pid_min_T->next;
				pid_min_T->next = tmpT;
			}

			newT = newT->next;
		}
	}

	newTmpP = players_head->next;

    while(newTmpP != players_head){

        newTmpT = newTmpP->tasks_head;

        printf("\tPlayer <%d,%d> =", newTmpP->pid, newTmpP->evidence);

	   while(newTmpT != newTmpP->tasks_sentinel){
            printf("<%d,%d> ", newTmpT->tid, newTmpT->difficulty);
            newTmpT = newTmpT->next;
        }
        printf("\n");
        newTmpP = newTmpP->next;
    }
	printf("\nDONE\n\n");

    return 1;
}


/**
 * @brief Give Away Work
 *
 * @return 1 on success
 *         0 on failure
 */
int give_work() {
	struct Players *pmax = NULL;
	struct Players *prevP = NULL;
	struct Players *tmpP = players_head->next;
	struct Players *pid_min =NULL;
	struct Tasks *pmaxT = NULL;
	struct Tasks *pid_min_T = NULL; 
	struct Tasks *tmpT = NULL;

	struct Tasks *prevT = NULL;

	int x;
	int TaskCount = 0;
	int minTaskCount = 0;
	int maxTaskCount = 1;

	//find the player with the least/max tasks
	while(tmpP != players_head){

		tmpT = tmpP->tasks_head;
		TaskCount = 0;

		while(tmpT != tmpP->tasks_sentinel){
			TaskCount++;
			tmpT = tmpT->next;
		}

		if(tmpP->tasks_head == tmpP->tasks_sentinel){
			pid_min = tmpP;
		}

		if(TaskCount <= minTaskCount){
			pid_min = tmpP;

			minTaskCount = TaskCount;
		}

		if(TaskCount >= maxTaskCount){
			pmax = tmpP;

			maxTaskCount = TaskCount;
		}

		tmpP = tmpP->next;
	}

	pid_min_T = pid_min->tasks_head;

	if(pmax != NULL){
		pmaxT = pmax->tasks_head;
	}
	else pmaxT = pid_min_T;

	//merge the two players' tasks lists together
	if(pid_min->tasks_head == pid_min->tasks_sentinel){

		for(x = 0; x < (maxTaskCount / 2); x++){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = pmaxT->tid;
			tmpT->difficulty = pmaxT->difficulty;

			pid_min->tasks_head = tmpT;
			pid_min->tasks_sentinel->next = tmpT;
			tmpT->next = pid_min->tasks_sentinel;

			pmaxT = pmaxT->next;
		}
	}
	else if(pid_min_T->difficulty == tmpT->difficulty){
		printf("\t1-");
		pmaxT = pmax->tasks_head;

		for(x = 0; x < (maxTaskCount / 2); x++){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = pmaxT->tid;
			tmpT->difficulty = pmaxT->difficulty;

			tmpT->next = pid_min->tasks_head;
			pid_min->tasks_head = tmpT;

			pmaxT = pmaxT->next;
		}
	}
	else{
		pmaxT = pmax->tasks_head;

		while(pid_min_T != pid_min->tasks_sentinel && pid_min_T->difficulty < tmpT->difficulty){
			pid_min_T = pid_min_T->next;
		}
		for(x = 0; x < (maxTaskCount / 2); x++){
			tmpT = (struct Tasks*)malloc(sizeof(struct Tasks));

			tmpT->tid = pmaxT->tid;
			tmpT->difficulty = pmaxT->difficulty;

			if(tmpT->difficulty <= pmax->tasks_head->difficulty){
				tmpT->next = pid_min->tasks_head;
				pid_min->tasks_head = tmpT;
			}
			else if(tmpT->difficulty > pmax->tasks_head->difficulty){
				tmpT->next = pid_min->tasks_sentinel;
				pid_min->tasks_sentinel->next->next = tmpT;
				pid_min->tasks_sentinel->next = tmpT;
			}
			else{
				tmpT->next = pid_min_T->next;
				pid_min_T->next = tmpT;
			}

			pmaxT = pmaxT->next;
		}
	}

	print_double_list();

    return 1;
}

/**
 * @brief Terminate
 *
 * @return 1 on success
 *         0 on failure
 */
int terminate() {
	struct Players *currP = players_head->next;

	int alienCnt = 0;
	int crewCnt = 0;
	int tasksCnt = 0;
	int x;

	for(x = 0; x < 3; x++){
		tasksCnt += tasks_head->tasks_count[x];
	}

	while(currP != players_head){
		if(currP->is_alien != 0){
			alienCnt++;
		}
		else crewCnt++;

		currP = currP->next;
	}

	if(crewCnt > alienCnt || tasksCnt == tasks_stack->count){
		printf("\tCrewmates win.\n");
	}
	else printf("Aliens win.\n");

	printf("DONE\n");
    return 1;
}

/**
 * @brief Print Players
 *
 * @return 1 on success
 *         0 on failure
 */
int print_players() {

    printf("\tPlayers = ");

    struct Players *tmp = players_head->next;

    while(tmp != players_head){
        printf(" <%d:%d>", tmp->pid, tmp->is_alien);
        tmp = tmp->next;
    }
    printf("\nDONE\n");
    return 1;
}

/**
 * @brief Print Tasks
 *
 * @return 1 on success
 *         0 on failure
 */
int print_tasks() {

	printf("\tList_Tasks = ");

    struct Tasks *tmp = tasks_head->head;

    while(tmp != NULL){
        printf(" <%d, %d>", tmp->tid, tmp->difficulty);
        tmp = tmp->next;
    }
    printf("\nDONE\n");

    return 1;
}

/**
 * @brief Print Stack
 *
 * @return 1 on success
 *         0 on failure
 */
int print_stack() {
	struct Tasks *tmpT = tasks_stack->head;

	printf("\tStack_tasks = ");

	while(tmpT != NULL){
		printf("<%d,%d> ", tmpT->tid, tmpT->difficulty);
		tmpT = tmpT->next;
	}
	printf("\nDONE\n");

    return 1;
}

/**
 * @brief Print Players & Task List
 *
 * @return 1 on success
 *         0 on failure
 */
int print_double_list() {

	struct Players *newTmpP = players_head->next;
	struct Tasks *newTmpT = NULL;

	while(newTmpP != players_head){

        newTmpT = newTmpP->tasks_head;

        printf("\tPlayer %d = ", newTmpP->pid);

	   while(newTmpT != newTmpP->tasks_sentinel){
            printf("<%d,%d> ", newTmpT->tid, newTmpT->difficulty);
            newTmpT = newTmpT->next;
        }
        printf("\n");
        newTmpP = newTmpP->next;
    }
	printf("\nDONE\n\n");

    return 1;
}

/**
 * @brief Free resources
 *
 * @return 1 on success
 *         0 on failure
 */

int free_all(void) {
	struct Players *tmpP = players_head->next;
    struct Tasks *tmpT = tasks_head->head;

    while(tmpP != players_head){
        free(tmpP);
        tmpP = tmpP->next;
    }

    while(tmpT!= NULL){
        free(tmpT);
        tmpT = tmpT->next;
    }

    tmpT = tasks_stack->head;

    while(tmpT!= NULL){
        free(tmpT);
        tmpT = tmpT->next;
    }

    tmpP = players_head->next;
    tmpT = tasks_head->head;

    while(tmpP != players_head){

       tmpT = tmpP->tasks_head;

	   while(tmpT != tmpP->tasks_sentinel){
            free(tmpT);
            tmpT = tmpT->next;
        }
        tmpP = tmpP->next;
    }
    
    printf("\nDONE\n");
	
    return 1;
}


        
int insert(SymbolTableEntry* new_entry){
    scopeList* tmp;
    scopeList* new_scope = (struct scopeList*) malloc(sizeof(struct scopeList));
    
    
    if(head == NULL){

        head = (struct scopeList*) malloc(sizeof(struct scopeList));

        new_scope->next = head->next;
        head->next = new_scope;
        sentinel = (struct scopeList*) malloc(sizeof(struct scopeList));
        sentinel = new_scope;

        tmp = head;
    else
        tmp = head;
        if(new_scope->value->scope > head->value->scope){
            new_scope->next = head->next;
            head->next = new_scope;
        }

        while(new_scope->value->scope != tmp->value->scope){
            tmp = tmp->next;
        }
    }

    SymbolTableEntry* start = tmp->scope_head;

    if(start == NULL){
        ftiaxnw stoixeio
    }
    else{
        while(start->next_in_scope != NULL){
            start = start->next_in_scope;
        }
        ftiaxnw stoixeio
    }
}
