/* 
 * Class Dice:
 * Pseudo Random 0 - 5
 */
class dice{
int x;
public:
  dice(int s = 0){
    srand(s);
    x = rand();
  }
  int recent(){
    return x;
  }
  int next(){
    return rand()%6;
  }  
};
