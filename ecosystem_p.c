#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NTHREADS 4

#define FREE 0  //Empty space
#define RABBIT 1 //Occupied by Rabbit 
#define FOX 2   //Occupied by Fox 
#define ROCK 3  //Occupied by Rock 

int current_gen = 0;

typedef struct{
    int occupied;
    int rabbit_proc;
    int fox_proc;
    int fox_food;
} ECO_CELL;

typedef struct{
    int GEN_PROC_RABITS;
    int GEN_PROC_FOXES;
    int GEN_FOOD_FOXES;
    int N_GEN;
    int rows;
    int cols;
} ECO_CONFIG;


ECO_CONFIG *newEcoConfig(){
    ECO_CONFIG *newConfig = (ECO_CONFIG *) malloc(sizeof(ECO_CONFIG));
    return newConfig;
}

ECO_CELL **globalBoard; //Board that will keep final generations
ECO_CELL **auxBoard;    //Board that will be modified
ECO_CONFIG *ecosystemGlobalConfig;  //Ecosystem configurations

/**
 * 
 * Util
*/

/**
 * Frees reserved memory for a matrix of cells
*/
void freeBoard(ECO_CELL** board) {
    /* free the memory - the first element of the array is at the start */
    free( &(board[0][0]));

    /* free the pointers into the memory */
    free(board);
    board = NULL;
}

/**
 * For DEBUG, prints board generation as objects (ex: *,R,F)
*/
void printBoard_generation(ECO_CELL **board){

    for (size_t i = 0; i < ecosystemGlobalConfig->cols+2; i++){ 
        printf("-");
    }

    printf("\n");

    for (size_t row = 0; row < ecosystemGlobalConfig->rows; row++){
        for (size_t col = 0; col < ecosystemGlobalConfig->cols; col++){
            
            if (col == 0){
                printf("|");
            }
            
            if(board[row][col].occupied == ROCK){
                printf("*");
            }else if (board[row][col].occupied == FOX){
                printf("F");
            }else if (board[row][col].occupied == RABBIT){
                printf("R");
            }else{
                printf(" ");
            }
            
            if (col == ecosystemGlobalConfig->cols-1){
                printf("|");
            }
            
        }
        printf("\n");
    }
    
    for (size_t i = 0; i < ecosystemGlobalConfig->cols+2; i++){ 
        printf("-");
    }
    printf("\n");
}

/**
 * For DEBUG, prints board generation with food values in place of the objects
*/
void printBoard_generation_food(ECO_CELL **board){

    for (size_t i = 0; i < ecosystemGlobalConfig->cols+2; i++){ 
        printf("-");
    }

    printf("\n");

    for (size_t row = 0; row < ecosystemGlobalConfig->rows; row++){
        for (size_t col = 0; col < ecosystemGlobalConfig->cols; col++){
            
            if (col == 0){
                printf("|");
            }
            
            if(board[row][col].occupied == ROCK){
                printf("*");
            }else if (board[row][col].occupied == FOX){
                printf("%d",board[row][col].fox_food);
            }else if (board[row][col].occupied == RABBIT){
                printf("R");
            }else{
                printf(" ");
            }
            
            if (col == ecosystemGlobalConfig->cols-1){
                printf("|");
            }
            
        }
        printf("\n");
    }
    
    for (size_t i = 0; i < ecosystemGlobalConfig->cols+2; i++){ 
        printf("-");
    }
    printf("\n");
}

/**
 * For DEBUG, prints board generation with procreation values in place of the objects
*/
void printBoard_generation_proc(ECO_CELL **board){

    for (size_t i = 0; i < ecosystemGlobalConfig->cols+2; i++){ 
        printf("-");
    }

    printf("\n");

    for (size_t row = 0; row < ecosystemGlobalConfig->rows; row++){
        for (size_t col = 0; col < ecosystemGlobalConfig->cols; col++){
            
            if (col == 0){
                printf("|");
            }
            
            if(board[row][col].occupied == ROCK){
                printf("*");
            }else if (board[row][col].occupied == FOX){
                printf("%d",board[row][col].fox_proc);
            }else if (board[row][col].occupied == RABBIT){
                printf("%d",board[row][col].rabbit_proc);
            }else{
                printf(" ");
            }
            
            if (col == ecosystemGlobalConfig->cols-1){
                printf("|");
            }
            
        }
        printf("\n");
    }
    
    for (size_t i = 0; i < ecosystemGlobalConfig->cols+2; i++){ 
        printf("-");
    }
    printf("\n");
}

/**
 * Adds ecosystem configurations to the global ecosystem config datatype
*/
void setGlobalCOnfig(int GEN_PROC_RABITS, int GEN_PROC_FOXES, int GEN_FOOD_FOXES, int N_GEN, int rows, int cols){
    ecosystemGlobalConfig->GEN_PROC_RABITS =  GEN_PROC_RABITS;
    ecosystemGlobalConfig->GEN_PROC_FOXES = GEN_PROC_FOXES;
    ecosystemGlobalConfig->GEN_FOOD_FOXES = GEN_FOOD_FOXES;
    ecosystemGlobalConfig->N_GEN = N_GEN;
    ecosystemGlobalConfig->rows = rows;
    ecosystemGlobalConfig->cols = cols;
}

/**
 * 
 * Allocates contiguous memory for the matrix of cells
*/
ECO_CELL **initBoard(){
    ECO_CELL *auxAll = (ECO_CELL *) malloc(ecosystemGlobalConfig->rows * ecosystemGlobalConfig->cols * sizeof(ECO_CELL));
    ECO_CELL **auxRows = (ECO_CELL **) malloc(ecosystemGlobalConfig->rows * sizeof(ECO_CELL*));

    for (int i=0; i<ecosystemGlobalConfig->rows; i++)
        auxRows[i] = &(auxAll[i*ecosystemGlobalConfig->rows]);

    for (size_t row = 0; row < ecosystemGlobalConfig->rows; row++){
        for (size_t col = 0; col < ecosystemGlobalConfig->cols; col++){
            auxRows[row][col].fox_food=0;
            auxRows[row][col].fox_proc=0;
            auxRows[row][col].occupied=FREE;
            auxRows[row][col].rabbit_proc=0;

        }
    }

    return auxRows;
}

/**
 * Prints the final output
*/
void print_output(){

    int nObjects=0;
    //Count objects in matrix
    for (int x = 0; x < ecosystemGlobalConfig->rows; x++){
        for (int y = 0; y < ecosystemGlobalConfig->cols; y++){
            if(globalBoard[x][y].occupied != FREE){ nObjects++; }
        }
    }
    printf("%d %d %d", ecosystemGlobalConfig->GEN_PROC_RABITS, ecosystemGlobalConfig->GEN_PROC_FOXES, ecosystemGlobalConfig->GEN_FOOD_FOXES);
    printf(" %d %d %d %d\n", ecosystemGlobalConfig->N_GEN-current_gen, ecosystemGlobalConfig->rows, ecosystemGlobalConfig->cols, nObjects);
    
    //Print each object
    for (int x = 0; x < ecosystemGlobalConfig->rows; x++){
        for (int y = 0; y < ecosystemGlobalConfig->cols; y++){
            if(globalBoard[x][y].occupied == ROCK){
                printf("ROCK %d %d\n", x, y);
            }else if (globalBoard[x][y].occupied == FOX){
                printf("FOX %d %d\n", x, y);
            }else if (globalBoard[x][y].occupied == RABBIT){
                printf("RABBIT %d %d\n", x, y);
            }

        }
    }
}

/**
 * 
 * Inputs
*/

void addObject(char *objline){
    char objectName[10];
    int objrow=-1;
    int objcol=-1;

    const char separator[2] = " ";
    char *token;
    int tokenCounter=0;

    //Split object string by empty spaces
    token = strtok(objline, separator); 
    tokenCounter++;
    while( token != NULL ) {
        //Object name
        if (tokenCounter == 1){
            strcpy(objectName, token);
        
        //Object row
        }else if (tokenCounter == 2){
            objrow = atoi(token);
        
        //Object column
        }else if (tokenCounter == 3){
            objcol = atoi(token);
        }

        token = strtok(NULL, separator);
        tokenCounter++;
    }

    //Add object to boards
    if(strcmp(objectName, "RABBIT") == 0){
        globalBoard[objrow][objcol].occupied = RABBIT;
        auxBoard[objrow][objcol].occupied = RABBIT;

    }else if(strcmp(objectName, "FOX") == 0){
        globalBoard[objrow][objcol].occupied = FOX;
        auxBoard[objrow][objcol].occupied = FOX;

    }else if(strcmp(objectName, "ROCK") == 0){
        globalBoard[objrow][objcol].occupied = ROCK;
        auxBoard[objrow][objcol].occupied = ROCK;

    }else{
        globalBoard[objrow][objcol].occupied = FREE;
        auxBoard[objrow][objcol].occupied = FREE;
    } 
}

void readInput(){
    int GEN_PROC_RABITS, GEN_PROC_FOXES, GEN_FOOD_FOXES;
    int N_GEN, rows, cols, nObjects;

    scanf("%d%d%d%d%d%d%d", &GEN_PROC_RABITS, &GEN_PROC_FOXES, &GEN_FOOD_FOXES, &N_GEN, &rows, &cols, &nObjects);

    //Save global configs
    ecosystemGlobalConfig = newEcoConfig();
    setGlobalCOnfig(GEN_PROC_RABITS, GEN_PROC_FOXES, GEN_FOOD_FOXES, N_GEN, rows, cols);

    //Allocate memory for boards
    globalBoard = initBoard();
    auxBoard = initBoard();

    char objLine[30];
    //Populate boards
    for (int i = 0; i < nObjects+1; i++){
        fgets(objLine, 30, stdin);
        if(i != 0){
            addObject(objLine);
        }
    }
}


/**
 * 
 * Game Util
*/


/** 
 * Funtion that returns available empty spaces around a given position (Up, Down, Left, Right)
*/
int count_empty_spaces(int x, int y){
    int count_empty_spaces=0; 

    if(y>0)
        if(globalBoard[x][y-1].occupied==FREE)
            count_empty_spaces++;

    if(y<ecosystemGlobalConfig->cols-1)
        if(globalBoard[x][y+1].occupied==FREE)
            count_empty_spaces++;

    if(x>0)
        if(globalBoard[x-1][y].occupied==FREE)
            count_empty_spaces++;
    if(x<ecosystemGlobalConfig->rows-1)        
        if(globalBoard[x+1][y].occupied==FREE)
            count_empty_spaces++;
    
    return count_empty_spaces;
}

/**
 * Function that return array of possible empty spaces, each position of the array represents a direction
 * [0] = North
 * [1] = East
 * [2] = South
 * [3] = West 
 * Each position will have 1 if the respective move in that direction is possible
*/
void get_free_space_coordinates(int x, int y, int *coordinates){
    for (size_t i = 0; i < 4; i++){
        coordinates[i]=0;
    }
    
    if(y>0){
        //West
        if(globalBoard[x][y-1].occupied==FREE){coordinates[3]=1;}
    }
    if(y<ecosystemGlobalConfig->cols-1){
        //East
        if(globalBoard[x][y+1].occupied==FREE){coordinates[1]=1;}
    }
    if(x>0){
        //North
        if(globalBoard[x-1][y].occupied==FREE){coordinates[0]=1;}
    }
    if(x<ecosystemGlobalConfig->rows-1){    
        //South 
        if(globalBoard[x+1][y].occupied==FREE){coordinates[2]=1;}
    }
}

/**
 * Copies new changes to the globalBoard
 * Used in the end of a generation to save all the changes of the respective generation
*/
void save_last_generation(){
    int rows = ecosystemGlobalConfig->rows;
    int cols = ecosystemGlobalConfig->cols;

    
    for (int x = 0; x < rows; x++){
        for (int y = 0; y < cols; y++){
        
            if(globalBoard[x][y].occupied!=ROCK){
                globalBoard[x][y].occupied = auxBoard[x][y].occupied;
                globalBoard[x][y].rabbit_proc = auxBoard[x][y].rabbit_proc;
                globalBoard[x][y].fox_proc = auxBoard[x][y].fox_proc;
                globalBoard[x][y].fox_food = auxBoard[x][y].fox_food;

            }
        }
    }


}

/**
 * 
 * Rabbits
*/

/**
 * Function that checks if a rabbit in a x,y position can procreate
 * returns 1 if rabbit can procreate, 0 if not
*/
int check_rabbit_procreate(int x, int y){
    int flag = 0;
    if(globalBoard[x][y].rabbit_proc >= ecosystemGlobalConfig->GEN_PROC_RABITS){
        flag = 1;
    }

    return flag;
}

/**
 * Function cheks if there is a rabbit at a new position newX,newY 
 * and if there is, checks if its procreate flag is bigger than the moving rabbit one
*/
int check_rabbit_conflict(int newX, int newY, int oldX, int oldY){
    int flag = 0;
    if(auxBoard[newX][newY].occupied==RABBIT && auxBoard[newX][newY].rabbit_proc > auxBoard[oldX][oldY].rabbit_proc){
        flag = 1;
    }
    return flag;
}

/**
 * Function that handles the movement of the rabbit at oldX,OldY to the new position at newX,newY
*/
void move_rabbit(int newX, int newY, int oldX, int oldY){
    //See if rabbit can procreate
    if(check_rabbit_procreate(oldX,oldY) == 1){
        //creat new rabbit & set proc flag to 0
        auxBoard[oldX][oldY].rabbit_proc=0;
        auxBoard[oldX][oldY].occupied=RABBIT;
        //move old rabbit & set proc flag to 0
        auxBoard[newX][newY].occupied=RABBIT;
        auxBoard[newX][newY].rabbit_proc=0;

    }else{
        //Check if other rabbir already at new location
        if(check_rabbit_conflict(newX, newY, oldX, oldY) == 1){
            //Better rabbit already at location = empty old location
            //empty old location
            auxBoard[oldX][oldY].rabbit_proc=0;
            auxBoard[oldX][oldY].occupied=FREE;
        }else{
            //move old rabbit & increment proc flag
            auxBoard[newX][newY].occupied=RABBIT;
            auxBoard[newX][newY].rabbit_proc = auxBoard[oldX][oldY].rabbit_proc + 1;
            //empty old location
            auxBoard[oldX][oldY].rabbit_proc=0;
            auxBoard[oldX][oldY].occupied=FREE;
        }
    }
}

void place_rabbit(int x, int y){

    int newX,newY;  //Coordinates of new position
    int spaces_to_move = count_empty_spaces(x,y);   //Nº of empty spaces to move    
    int possible_moves[4];  //Availablew moving directions  [0] = North [1] = East [2] = South [3] = West 
    get_free_space_coordinates(x,y, possible_moves);

    if(spaces_to_move==0){
        //Stays in position
        auxBoard[x][y].rabbit_proc++;
    }else if(spaces_to_move == 1){

        if(possible_moves[0] == 1){
            //North
            newX = x-1;
            newY = y;
        }else if(possible_moves[1]==1){
            //East
            newX = x;
            newY = y+1;
        }else if(possible_moves[2]==1){
            //South
            newX = x+1;
            newY = y;
        }else if(possible_moves[3]==1){
            //West
            newX = x;
            newY = y-1;
        }
        move_rabbit(newX, newY, x, y);

    }else{
        //handle conflict
        int move_decider = (int) (current_gen + x + y) % spaces_to_move;
        //decide: 2 = West --> possible_moves[3] = West
        //decide: 1 = East --> possible_moves[1] = East
        //decide: 3 = North --> possible_moves[0] = North
        //decide: 0 = South --> possible_moves[2] = South
        //Need to check if object can be moved to the decider direction
        if(move_decider == 2 && possible_moves[3] == 1){
            //West
            newX = x;
            newY = y-1;
            move_rabbit(newX, newY, x, y);
        }else if(move_decider == 1 && possible_moves[1]==1){
            //East
            newX = x;
            newY = y+1;
            move_rabbit(newX, newY, x, y);
        }else if(move_decider == 3 && possible_moves[0]==1){
            //North
            newX = x-1;
            newY = y;
            move_rabbit(newX, newY, x, y);
        }else if(move_decider == 0 && possible_moves[2]==1){
            //South
            newX = x+1;
            newY = y;
            move_rabbit(newX, newY, x, y);
        }else if(possible_moves[0]==1){
            //If all above fail, move clockwise starting in North->0, East->1, South->2, West-> 3, if possible
            //North
            newX = x-1;
            newY = y;
            move_rabbit(newX, newY, x, y);
        }else if(possible_moves[1]==1){
            //East
            newX = x;
            newY = y+1;
            move_rabbit(newX, newY, x, y);
        }else if(possible_moves[2]==1){
            //South
            newX = x+1;
            newY = y;
            move_rabbit(newX, newY, x, y);
        }else if(possible_moves[3]==1){
            //West
            newX = x;
            newY = y-1;
            move_rabbit(newX, newY, x, y);
        }
    }
}

/**
 * Start moving rabbits in the board
*/
void init_rabbit_turn(){
    int rows = ecosystemGlobalConfig->rows;
    int cols = ecosystemGlobalConfig->cols;
    int x,y;

    for (x = 0; x < rows; x++){
        for (y = 0; y < cols; y++){
            if(globalBoard[x][y].occupied==RABBIT){
                place_rabbit(x,y);
            }
        }
    }
}


/**
 * 
 * Foxes
*/

/**
 * Function that checks if a fox in a x,y position can procreate
 * returns 1 if fox can procreate, 0 if not
*/
int check_fox_procreate(int x, int y){
    int flag = 0;
    if(globalBoard[x][y].fox_proc >= ecosystemGlobalConfig->GEN_PROC_FOXES){
        flag = 1;
    }
    return flag;
}

/**
 * Function that checks if a fox in a x,y position is at max hunger
 * returns 1 if fox is at max hunger, 0 if not
*/
int check_fox_hungry(int x, int y){
    int flag = 0;
    if(globalBoard[x][y].fox_food >= ecosystemGlobalConfig->GEN_FOOD_FOXES){
        flag = 1;
    }
    return flag;
}

/** 
 * Funtion that returns available rabbit spaces around a given position (Up, Down, Left, Right)
*/
int count_food_spaces(int x, int y){

    int count_food_spaces=0; 

    if(y>0)
        if(globalBoard[x][y-1].occupied==RABBIT)
            count_food_spaces++;

    if(y<ecosystemGlobalConfig->cols-1)
        if(globalBoard[x][y+1].occupied==RABBIT)
            count_food_spaces++;

    if(x>0)
        if(globalBoard[x-1][y].occupied==RABBIT)
            count_food_spaces++;
    if(x<ecosystemGlobalConfig->rows-1)        
        if(globalBoard[x+1][y].occupied==RABBIT)
            count_food_spaces++;
    
    return count_food_spaces;
}

/**
 * Function that return array of possible rabbit spaces, each position of the array represents a direction
 * [0] = North
 * [1] = East
 * [2] = South
 * [3] = West 
 * Each position will have 1 if the respective move in that direction is possible
*/
void get_food_space_coordinates(int x, int y, int *coordinates){
    //int coordinates[4]; [0] = North, [1] = East, [2] = South, [3] = West 

    for (size_t i = 0; i < 4; i++){
        coordinates[i]=0;
    }
    
    if(y>0){
        //West
        if(globalBoard[x][y-1].occupied==RABBIT){coordinates[3]=1;}
    }
    if(y<ecosystemGlobalConfig->cols-1){
        //East
        if(globalBoard[x][y+1].occupied==RABBIT){coordinates[1]=1;}
    }
    if(x>0){
        //North
        if(globalBoard[x-1][y].occupied==RABBIT){coordinates[0]=1;}
    }
    if(x<ecosystemGlobalConfig->rows-1){    
        //South 
        if(globalBoard[x+1][y].occupied==RABBIT){coordinates[2]=1;}
    }
}

/**
 * Function cheks if there is a fox at a new position newX,newY 
 * and if there is:
 *  -checks if its procreate flag is bigger than the moving fox one
 *  -and if procreate flag is equal, checks if fox at position is less hungry than the moving one
 * 
 * Returns 1 if fox at new position is better
*/
int check_fox_conflict(int newX, int newY, int oldX, int oldY){
    int flag = 0;

    if(auxBoard[newX][newY].occupied==FOX && auxBoard[newX][newY].fox_proc > auxBoard[oldX][oldY].fox_proc){
        flag = 1;
    }else if(auxBoard[newX][newY].occupied==FOX && auxBoard[newX][newY].fox_proc == auxBoard[oldX][oldY].fox_proc && auxBoard[newX][newY].fox_food < auxBoard[oldX][oldY].fox_food){
        flag = 1;
    }

    return flag;
}

/**
 * Function that handles the movement of the fox, FOR EMPTY SPACES, at oldX,OldY to the new position at newX,newY
*/
void move_fox(int newX, int newY, int oldX, int oldY){

    //First see if fox is at max hunger
    if(check_fox_hungry(oldX, oldY) == 1){
        //fox dies from not eating
        auxBoard[oldX][oldY].fox_proc=0;
        auxBoard[oldX][oldY].occupied=FREE;
        auxBoard[oldX][oldY].fox_food=0;

    //Handle procreation
    }else if(check_fox_procreate(oldX,oldY) == 1){

        //move old rabbit & set proc flag to 0
        auxBoard[newX][newY].occupied=FOX;
        auxBoard[newX][newY].fox_proc=0;
        auxBoard[newX][newY].fox_food = auxBoard[oldX][oldY].fox_food + 1;

        //creat new fox & set proc flag to 0
        auxBoard[oldX][oldY].fox_proc=0;
        auxBoard[oldX][oldY].occupied=FOX;
        auxBoard[oldX][oldY].fox_food=0;
        
    }else{
        //Check if there is other fox already at new position
        if(check_fox_conflict(newX, newY, oldX, oldY) == 1){
            //Better fox already at location = empty old location
            auxBoard[oldX][oldY].fox_proc=0;
            auxBoard[oldX][oldY].occupied=FREE;
            auxBoard[oldX][oldY].fox_food=0;
        }else{
            //move old fox & increment proc flag
            auxBoard[newX][newY].occupied=FOX;
            auxBoard[newX][newY].fox_proc = auxBoard[oldX][oldY].fox_proc + 1;
            auxBoard[newX][newY].fox_food = auxBoard[oldX][oldY].fox_food + 1;
            //empty old location
            auxBoard[oldX][oldY].fox_proc=0;
            auxBoard[oldX][oldY].occupied=FREE;
            auxBoard[oldX][oldY].fox_food=0;
        }
    }
}

/**
 * Function that handles the movement of the fox, FOR RABBIT SPACES, at oldX,OldY to the new position at newX,newY
*/
void move_fox_eat(int newX, int newY, int oldX, int oldY){
    
    //First see if fox is at max hunger
    if(check_fox_procreate(oldX,oldY) == 1){
        //move old rabbit & set proc flag to 0
        auxBoard[newX][newY].occupied = FOX;
        auxBoard[newX][newY].fox_proc = 0;
        auxBoard[newX][newY].fox_food = 0;

        //creat new fox & set proc flag to 0
        auxBoard[oldX][oldY].fox_proc = 0;
        auxBoard[oldX][oldY].occupied = FOX;
        auxBoard[oldX][oldY].fox_food = 0;
    }else{
        
        //Check if another fox is already there
        if(check_fox_conflict(newX, newY, oldX, oldY) == 1){
            //empty old location
            auxBoard[oldX][oldY].fox_proc=0;
            auxBoard[oldX][oldY].occupied=FREE;
            auxBoard[oldX][oldY].fox_food=0;
        }else{
            //move old fox & increment proc flag
            auxBoard[newX][newY].occupied=FOX;
            auxBoard[newX][newY].fox_proc = auxBoard[oldX][oldY].fox_proc + 1;
            auxBoard[newX][newY].fox_food = 0;
            //empty old location
            auxBoard[oldX][oldY].fox_proc=0;
            auxBoard[oldX][oldY].occupied=FREE;
            auxBoard[oldX][oldY].fox_food=0;
        }
        
    }
}

void place_fox(int x, int y){

    int newX,newY;  //Coordinates of new position
    int spaces_to_move = count_empty_spaces(x,y);   //Nº of empty spaces to move
    int spaces_to_eat = count_food_spaces(x,y); //Nº of eat spaces to move

    int possible_empty_moves[4];    //Availablew moving directions for empty spaces  [0] = North [1] = East [2] = South [3] = West 
    get_free_space_coordinates(x,y, possible_empty_moves);

    int possible_eat_moves[4];  //Availablew moving directions for eating spaces  [0] = North [1] = East [2] = South [3] = West 
    get_food_space_coordinates(x,y, possible_eat_moves);

    
    //handle eating first
    if(spaces_to_eat>0){
        if(spaces_to_eat == 1){

            if(possible_eat_moves[0] == 1){
                //North
                newX = x-1;
                newY = y;
            }else if(possible_eat_moves[1]==1){
                //East
                newX = x;
                newY = y+1;
            }else if(possible_eat_moves[2]==1){
                //South
                newX = x+1;
                newY = y;
            }else if(possible_eat_moves[3]==1){
                //West
                newX = x;
                newY = y-1;
            }
            move_fox_eat(newX, newY, x, y);

        }else{
            //handle conflict
            int move_decider = (int) (current_gen + x + y) % spaces_to_eat;
            //decide: 2 = West --> possible_eat_moves[3] = West
            //decide: 1 = East --> possible_eat_moves[1] = East
            //decide: 3 = North --> possible_eat_moves[0] = North
            //decide: 0 = South --> possible_eat_moves[2] = South
            //Need to check if object can be moved to the decider direction
            if(move_decider == 2 && possible_eat_moves[3] == 1){
                //West
                newX = x;
                newY = y-1;
                move_fox_eat(newX, newY, x, y);
            }else if(move_decider == 1 && possible_eat_moves[1]==1){
                //East
                newX = x;
                newY = y+1;
                move_fox_eat(newX, newY, x, y);
            }else if(move_decider == 3 && possible_eat_moves[0]==1){
                //North
                newX = x-1;
                newY = y;
                move_fox_eat(newX, newY, x, y);
            }else if(move_decider == 0 && possible_eat_moves[2]==1){
                //South
                newX = x+1;
                newY = y;
                move_fox_eat(newX, newY, x, y);
            }else if(possible_eat_moves[0]==1){
                //If all above fail, move clockwise starting in North->0, East->1, South->2, West-> 3, if possible
                //North
                newX = x-1;
                newY = y;
                move_fox_eat(newX, newY, x, y);
            }else if(possible_eat_moves[1]==1){
                //East
                newX = x;
                newY = y+1;
                move_fox_eat(newX, newY, x, y);
            }else if(possible_eat_moves[2]==1){
                //South
                newX = x+1;
                newY = y;
                move_fox_eat(newX, newY, x, y);
            }else if(possible_eat_moves[3]==1){
                //West
                newX = x;
                newY = y-1;
                move_fox_eat(newX, newY, x, y);
            }

        }
    }else{
        //nothing to eat movement
        if(spaces_to_move==0){
            //Check hungry treshold before anything
            if(check_fox_hungry(x,y)==1){
                //Fox fox dies from not eating
                auxBoard[x][y].fox_proc=0;
                auxBoard[x][y].occupied=FREE;
                auxBoard[x][y].fox_food=0;
            }else{
                //Stays in same cell & increments hungry and prociation flags
                auxBoard[x][y].fox_proc++;
                auxBoard[x][y].fox_food++;
            }
        }else if(spaces_to_move == 1){

            if(possible_empty_moves[0] == 1){
                //North
                newX = x-1;
                newY = y;
            }else if(possible_empty_moves[1]==1){
                //East
                newX = x;
                newY = y+1;
            }else if(possible_empty_moves[2]==1){
                //South
                newX = x+1;
                newY = y;
            }else if(possible_empty_moves[3]==1){
                //West
                newX = x;
                newY = y-1;
            }
            move_fox(newX, newY, x, y);

        }else{
            //handle conflict
            int move_decider = (int) (current_gen + x + y) % spaces_to_move;
            //decide: 2 = West --> possible_moves[3] = West
            //decide: 1 = East --> possible_moves[1] = East
            //decide: 3 = North --> possible_moves[0] = North
            //decide: 0 = South --> possible_moves[2] = South
            //Need to check if object can be moved to the decider direction
            
            if(move_decider == 2 && possible_empty_moves[3] == 1){
                //West
                newX = x;
                newY = y-1;
                move_fox(newX, newY, x, y);
            }else if(move_decider == 1 && possible_empty_moves[1]==1){
                //East
                newX = x;
                newY = y+1;
                move_fox(newX, newY, x, y);
            }else if(move_decider == 3 && possible_empty_moves[0]==1){
                //North
                newX = x-1;
                newY = y;
                move_fox(newX, newY, x, y);
            }else if(move_decider == 0 && possible_empty_moves[2]==1){
                //South
                newX = x+1;
                newY = y;
                move_fox(newX, newY, x, y);
            }else if(possible_empty_moves[0]==1){
                //If all above fail, move clockwise starting in North->0, East->1, South->2, West-> 3, if possible
                //North
                newX = x-1;
                newY = y;
                move_fox(newX, newY, x, y);
            }else if(possible_empty_moves[1]==1){
                //East
                newX = x;
                newY = y+1;
                move_fox(newX, newY, x, y);
            }else if(possible_empty_moves[2]==1){
                //South
                newX = x+1;
                newY = y;
                move_fox(newX, newY, x, y);
            }else if(possible_empty_moves[3]==1){
                //West
                newX = x;
                newY = y-1;
                move_fox(newX, newY, x, y);
            }
        }
    }
}

/**
 * Start moving foxes in the board
*/
void init_fox_turn(){
    int rows = ecosystemGlobalConfig->rows;
    int cols = ecosystemGlobalConfig->cols;
    int x,y;

    for (x = 0; x < rows; x++){
        for (y = 0; y < cols; y++){
            
            if(globalBoard[x][y].occupied==FOX){
                place_fox(x,y);
            }
        }
    }
}



/**
 * 
 * Main
*/

int main (int argc, char *argv[]) {

    double total_start = omp_get_wtime();
    readInput();

    double computations_start = omp_get_wtime();
    for (int i = 0; i < ecosystemGlobalConfig->N_GEN; i++){

        init_rabbit_turn();
        init_fox_turn();
        save_last_generation();
        current_gen++;

        //printf("GEN %d\n", i);
        //printBoard_generation(globalBoard);
        //printBoard_generation_food(globalBoard);
        //printBoard_generation_proc(globalBoard);
        //printBoard_generation(auxBoard);
    }
    double computations_finish = omp_get_wtime();

    print_output();
    double total_finish = omp_get_wtime();

    printf("Computations execution time: %f\n", computations_finish-computations_start);
    printf("I/O execution time: %f\n", (total_finish-total_start) - (computations_finish-computations_start));
    printf("Total execution time: %f\n", total_finish-total_start);

    //Free reserved memory
    free(globalBoard);
    free(auxBoard);
    return 0;
}