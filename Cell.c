/**
* \file List.c
* \author Nicolas Le Gall
* \date 11/12/2014
* \brief Definit l'ensemble des methodes pour une structure de type Cell
*/

#include "Cell.h"

operation_t op[4];

/**
Initialisation du tableau des operations
*/
void init() {
	op[0].nom = "+";
	op[0].ptr = &addition;

	op[1].nom = "-";
	op[1].ptr = &subtraction;

	op[2].nom = "*";
	op[2].ptr = &multiplication;

	op[3].nom = "/";
	op[3].ptr = &division;
}

/**
Evaluation du contenu de la cellule 
On decoupe la formule saisie dans la cellule 
afin d'evaluer chaque partie. On applique 
ensuite la bonne fonction : un nombre (double) 
ou une operation (operator).
* \param worksheet Adresse du worksheet
* \param cell Adresse de la cellule a evaluer
*/
void evaluate(worksheet_t* worksheet, cell_t* cell) {
	char* explode;
	char* string;
	int i;
	/* Pile pour calcule la valeur de la cellule */
	pile_t pile;
	pile_t* ptr_pile;
	/* Liste des tokens pour recalculer la cellule par la suite */
	list_t list;
	list_t* ptr_list;

	cell_t* dependance;

	ptr_pile = &pile;
	ptr_list = &list;

	initPile(ptr_pile, strlen(cell->saisi));
	ptr_list = initList();

	string = strdup(cell->saisi);			/* On copie la formule */
	explode = strtok(string, " ");			/* On decompose la formule */

	while(explode != NULL) {				/* Tant que explode n'est pas NULL */
		if (strtod(explode, NULL) != 0.00)	/* Si c'est un nombre de type double */
		{
			/* Creation d'un token de type double et insertion dans la pile */
			token_t* tmp;
			tmp = newDoubleToken(strtod(explode, NULL));
			ptr_list = insertHead(ptr_list, tmp);
			stack(ptr_pile, strtod(explode, NULL));
		}
		else
		{
			dependance = getReference(worksheet->cells, explode);
			if (dependance != NULL)			/* Si les dependances sont pas NULL */
			{
				/* On ajoute a la cellule la reference de sa dependance
				et dans la depence qu'elle est utilisee par la cellule */
				cell->dependancies = insertHead(cell->dependancies, dependance);
				dependance->usedBy = insertHead(dependance->usedBy, cell);
				stack(ptr_pile, dependance->value);
			}
			else {							/* Sinon (operation) */
				for (i = 0; i < 4; ++i)		/* On parcourt le tableau pour trouver la bonne */
				{
					if (strcmp(explode, op[i].nom) == 0)
					{
						/* On cree un nouveau token operator et on applique l'operation sur la pile */
						token_t* tmp;
						tmp = newOperatorToken(op[i].ptr);
						ptr_list = insertHead(ptr_list, tmp);
						op[i].ptr(ptr_pile);
					}
				}
			}
		}
		explode = strtok(NULL, " ");		/* On passe a l'element suivant dans la formule */
	}
	cell->value = unstack(ptr_pile);		/* On recupere l'element dans la pile calculee */
	if (cell->value == -1)					/* Si '-1', on remet a 0 */
	{
		cell->value = 0;
	}
	cell->tokens = ptr_list;				/* On donne l'adresse de la liste des tokens a la cellule */
}

/**
Evaluation du contenu des cellules de la feuille 
de calcul en tenant compte des dependances
* \param worksheet Adresse du worksheet
*/
void evaluateGraph(worksheet_t* worksheet) {
	list_t* tmp = worksheet->graphDep;
	while(tmp) {
		evaluate(worksheet, tmp->value);
		tmp = tmp->next;
	}
}

/**
Creer un nouveau token pour l'insertion dans la
liste de type double
* \param val Double a transformer en token
* \return Adresse en memoire du token
*/
token_t* newDoubleToken(double val) {
	token_t* token;

	token = (token_t*)malloc(sizeof(token_t));

	token->type = VALUE;
	token->value.cst = val;

	return token;
}

/**
Creer un nouveau token pour l'insertion dans la
liste de type operation
* \param ptr Adresse de l'operation
* \return Adresse en memoire du token
*/
token_t* newOperatorToken(void (*ptr)(pile_t* eval)) {
	token_t* token;

	token = (token_t*)malloc(sizeof(token_t));

	token->type = OPERATOR;
	token->value.ptr = ptr;

	return token;
}

/**
Additionne les deux derniers elements de
la pile
* \param eval Adresse de la pile
*/
void addition(pile_t* eval) {
	double res;
	double tmp;
	double tmp2;

	tmp = unstack(eval);
	tmp2 = unstack(eval);
	if ((tmp == -1)||(tmp2 == -1))
	{
		return;
	}
	res = tmp + tmp2;
	stack(eval, res);
}

/**
Soustrait les deux derniers elements de
la pile
* \param eval Adresse de la pile
*/
void subtraction(pile_t* eval) {
	double res;
	double tmp;
	double tmp2;

	tmp = unstack(eval);
	tmp2 = unstack(eval);
	if ((tmp == -1)||(tmp2 == -1))
	{
		return;
	}
	res = tmp - tmp2;
	stack(eval, res);
}

/**
Multiplie les deux derniers elements de
la pile
* \param eval Adresse de la pile
*/
void multiplication(pile_t* eval) {
	double res;
	double tmp;
	double tmp2;

	tmp = unstack(eval);
	tmp2 = unstack(eval);
	if ((tmp == -1)||(tmp2 == -1))
	{
		return;
	}
	res = tmp * tmp2;
	stack(eval, res);
}

/**
Divise les deux derniers elements de
la pile
* \param eval Adresse de la pile
*/
void division(pile_t* eval) {
	double res;
	double tmp;
	double tmp2;

	tmp = unstack(eval);
	tmp2 = unstack(eval);
	if ((tmp == -1)||(tmp2 == -1))
	{
		return;
	}
	res = tmp / tmp2;
	stack(eval, res);
}

/**
Affiche l'ensemble des tokens
* \param list Adresse de la liste
*/
void viewListCell(list_t* list) {
	list_t* tmp = list;
	int i;
	i = 0;
	while(tmp) {
		if (((token_t*)(tmp->value))->type == 0)
			printf("Element %d : %f\n", i, ((token_t*)(tmp->value))->value.cst);
		else
			printf("Element %d : %p\n", i, (void*)((token_t*)(tmp->value))->value.ref);
		tmp = tmp->next;
		i++;
	}
}

/**
Affiche l'ensemble des adresses des cellules
et leur nom
* \param list Adresse de la liste
*/
void viewCells(list_t* list) {
	list_t* tmp = list;
	while(tmp) {
		printf("Case : %s - Value : %p\n", ((cell_t*)tmp->value)->name, tmp->value);
		tmp = tmp->next;
	}
}

/**
Tri des cellules dans l'ordre de leurs dependances 
(du plus bas vers le plus eleve)
* \param worksheet Adresse du worksheet
*/
void topologicalSorting(worksheet_t* worksheet) {
	list_t* tmp = worksheet->cells; /* Permet de se deplacer dans la liste */
	list_t* tmpDep;					/* Liste des dependances de la cellule */
	list_t* shortCells;				/* Liste des cellules triees */

	void* cellDep;
	int nbCells;
	int insertions;
	int cmpDep;
	int depFound;

	nbCells = 0;
	insertions = 0;
	shortCells = initList();

	/* Nombre des cellules dans la feuille */
	while(tmp) {
		nbCells += 1;
		tmp = tmp->next;
	}
	/* Boucle tant que le nombre d'insertions n'est pas equel au nombre de cellules */
	while(nbCells != insertions) {
		tmp = worksheet->cells;		/* On recommence au debut afin de parcourir toutes les cellules */
		while(tmp) {
			/* Si la cellule possede des dependances */
			if (((cell_t*)tmp->value)->dependancies != NULL)
			{
				tmpDep = ((cell_t*)tmp->value)->dependancies;
				cmpDep = 0;
				depFound = 0;
				while(tmpDep) {
					/* Pour eviter d'avoir deux fois la meme cellule */
					cellDep = getReference(shortCells, ((cell_t*)tmpDep->value)->name);
					if (cellDep != NULL)
					{
						depFound += 1;
					}
					cmpDep += 1;
					tmpDep = tmpDep->next;
				}
				if (cmpDep == depFound)
				{
					shortCells = insertQueue(shortCells, tmp->value);
					insertions += 1;
					break;
				}
			}
			/* Si la cellule ne possede pas de dependances mais est utilisee par d'autres cellules */
			else if ((((cell_t*)tmp->value)->dependancies == NULL)&&((cell_t*)tmp->value)->usedBy != NULL)
			{
				/* Pour eviter d'avoir deux fois la meme cellule */
				cellDep = getReference(shortCells, ((cell_t*)tmp->value)->name);
				if (cellDep == NULL)
				{
					shortCells = insertQueue(shortCells, tmp->value);
					insertions += 1;
					break;
				}
			}
			/* Si la cellule est libre des autres cellule */
			else if (((cell_t*)tmp->value)->dependancies == NULL)
			{
				/* Pour eviter d'avoir deux fois la meme cellule */
				cellDep = getReference(shortCells, ((cell_t*)tmp->value)->name);
				if (cellDep == NULL)
				{
					shortCells = insertQueue(shortCells, tmp->value);
					insertions += 1;
					break;
				}
			}
			tmp = tmp->next;
		}
	}
	worksheet->graphDep = shortCells;
}

/**
Calcul pour chaque cellule son degre de dependance
* \param worksheet Adresse du worksheet
*/
void getDegree(worksheet_t* worksheet) {
	list_t* tmp = worksheet->cells;
	list_t* tmp2;
	int cmp;
	while(tmp) {
		cmp = 0;
		if (((cell_t*)(tmp->value))->dependancies != NULL)
		{
			tmp2 = ((cell_t*)(tmp->value))->dependancies;
			while(tmp2) {
				++cmp;
				tmp2 = tmp2->next;
			}
		}
		((cell_t*)(tmp->value))->degree = cmp;
		tmp = tmp->next;
	}
}

/**
Retourne la reference d'une cellule a partir de
son nom
* \param worksheet Adresse du worksheet
* \param target Le nom de la cellule
* \return Adresse de la cellule ou NULL sinon
*/
void* getReference(list_t* list, char* name) {
	list_t* tmp = list;
	while(tmp) {
		if (strcmp(((cell_t*)tmp->value)->name, name) == 0)
		{
			return tmp->value;
		}
		tmp = tmp->next;
	}
	return NULL;
}