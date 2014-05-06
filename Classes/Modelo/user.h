#ifndef  _USER_H_
#define  _USER_H_

#include <vector>
#include "card.h"

using namespace std;

class user{
  int nLife;
  int nBullet;
  int nZombie;
  int leftMoves;
  vector<card> handCardVector;
  vector<card> tableCardVector;
public:
  /*
   * Create an empty user
   */
  user();
  /*
   * Increase a life point
   */
  void increaseLife();
  /*
   * Decrease a life point
   */
  void decreaseLife();
  /*
   * Increaase a bullet point
   */
  void increaseBullet();
  /*
   * Decrease a bullet point
   */
  void decreaseBullet();
  /*
   * Increase a zombie point
   */
  void increaseZombie();
  /*
   * Decrease a zombie point
   */
  void decreaseZombie();
  /*
   * Return the number of life points
   */
  int getLife();
  /*
   * Return the number of bullet points
   */
  int getBullet();
  /*
   * Return the number of zombie points
   */
  int getZombie();
  /*
   * Push the card ca in the list of the player
   */
  void pushHandCard(card ca);
  /*
   * Get the card idCard
   */
  card& getHandCard(int idCard);
  /*
   * Erase the card idCard of the hand
   */
  bool eraseHandCard(int idCard);
  /*
   * Set x left moves
   */
  void setLeftMoves(int x);
  /*
   * Get the number of left moves
   */
  int getLeftMoves();
  /*
   * Init the user
   */
  void init();
  /*
   * Init the user after get killed
   */
  void die();
};

#endif