/*
 * PlayScene.cpp
 *
 *  Created on: 03/04/2014
 *      Author: hector
 */

#include "MenuScene.h"
#include "VisibleRect.h"
#include "support/CCPointExtension.h"
#include "SimpleAudioEngine.h"
#include <cmath>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

CCScene* PlayScene::scene()
{
	CCLOG("Entrando a: CCScene* PlayScene::scene()\n");
    CCScene *scene = CCScene::create();
    PlayScene *layer = PlayScene::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool PlayScene::init()
{
	CCLOG("Entrando a: bool PlayScene::init()\n");
    if ( !CCLayer::init() )
    {
        return false;
    }

    GameState.init(NUM_OF_USERS, NUM_OF_MACHINES);

    CCSprite * mapCardSprite = CCSprite::create(mapPath[0].c_str());
    PIXELS_MAP_CARD = mapCardSprite->getTextureRect().size.height;
    PIXELS_TILE = PIXELS_MAP_CARD/3.0;

    boxMapCardSize = CCSizeMake(PIXELS_MAP_CARD, PIXELS_MAP_CARD);

    for (int i = 0; i < NUM_OF_PLAYER; i++)
    	mSprite[i] = CCSprite::create(PATH_PLAYER_SPRITE[i].c_str());

    CCSpriteFrameCache* cache = CCSpriteFrameCache::sharedSpriteFrameCache();

    for (int i = 0; i < NUM_OF_PLAYER; i++){
    	for (int j = 0; j < 4; j++)
    		cache->addSpriteFramesWithFile(("sprite/"+NAME_PLAYER_SPRITE_ANIMATION[i][j]+".plist").c_str(),
                                           ("sprite/"+NAME_PLAYER_SPRITE_ANIMATION[i][j]+".png").c_str());
    }

    for (int i = 0; i < 4; i++){
		cache->addSpriteFramesWithFile(("sprite/"+NAME_ZOMBIE_SPRITE_ANIMATION[i]+".plist").c_str(),
									   ("sprite/"+NAME_ZOMBIE_SPRITE_ANIMATION[i]+".png").c_str());
    }

    // Agrega dados a cache
    cache->addSpriteFramesWithFile("sprite/movedicered.plist", "sprite/movedicered.png");
    cache->addSpriteFramesWithFile("sprite/movediceblue.plist", "sprite/movediceblue.png");

    // Inicializa Nombre de las rondas
    name[0][0] = "Agarra Carta del Mazo";
    name[0][1] = "Coloca la carta de mapa obtenida";
    name[1][0] = "Lanza el dado Azul para Moverte";
    name[1][1] = "Mueve tu personaje";
	name[2][0] = "Lanza el dado Rojo para mover Zombies";
	name[2][1] = "Mueve los zombies";

    // Inicia variables:
    _moveLayer = CCLayerPanZoom::create();
    _stayLayer = CCLayer::create();

    touches = 0;
    lastRedDiceResult = 5;
    lastBlueDiceResult = 5;
    WAIT = false;
    PUTMAPCARD = false;
    currFase = 0;
    currSubFase = 0;
    fase[0] = true;
    currCardRotation = 0;
    gameIsOver = false;

    for (int i = 1; i < NUM_FASES; i++) fase[i] = false;

    // Coloca elementos sobre layer estatico
    setMenuBackMenu(_stayLayer);
//    setOptionsLifeMenu(_stayLayer); // provisional
//    setOptionsBulletMenu(_stayLayer); // provisional
    setDices(_stayLayer);
    setInterface();
    setPlayerInfo();
    setBackgrounds();
    setPickMapCard();

    // Coloca elementos sobre el layer dinamico
    setMap(_moveLayer);
    setSprites(_moveLayer);

    // Habilita eventos touch
    this->setTouchEnabled( true );

    CCPoint centro = mapCardMatrixToAxis(MAX_MAP_DIM/2,MAX_MAP_DIM/2);
    centro.x -= VisibleRect::center().x;
    centro.y -= VisibleRect::center().y;
    _moveLayer->setPosition(-centro);

	_moveLayer->setAnchorPoint(ccp(0.0, 0.0));
	_moveLayer->setContentSize(CCSize((MAX_MAP_DIM+2)*PIXELS_MAP_CARD, (MAX_MAP_DIM+2)*PIXELS_MAP_CARD));

	_moveLayer->setPanBoundsRect(CCRectMake(VisibleRect::getVisibleRect().origin.x,
							     	 	 	VisibleRect::getVisibleRect().origin.y,
							     	 	 	VisibleRect::getVisibleRect().size.width,
							     	 	 	VisibleRect::getVisibleRect().size.height));

    // Añade layers al layer principal
    addChild(_moveLayer);
    addChild(_stayLayer);

    // Se prepara la primera fase del turno
    showFirstPhase();

    return true;
}

void PlayScene::onEnter() {
    CCLayer::onEnter();

    CCLOG("onEnter()");
    if (GameState.isCurrentPlayerMachine())
    	firstPhase(ccp(-1,-1), ccp(-1,-1));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * *
 *           Funciones Callbacks                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

void PlayScene::skipMenuCallback(CCObject* pSender)
{
	WAIT = true;

	if (currFase == 1)
		removePlayerBox();
	if (currFase == 2)
		removeZombieBox();

	changePhase(NULL, NULL);

	WAIT = false;
}

void PlayScene::mainMenuCallback(CCObject* pSender)
{
	CCLOG("Entrando a: void PlayScene::mainMenuCallback(CCObject* pSender)\n");
	CCScene* pScene = MenuScene::scene();
	CCDirector::sharedDirector()->replaceScene(pScene);
}

void PlayScene::incrementLifeMenuCallback(CCObject* pSender)
{
	CCLOG("Entrando a: void PlayScene::incrementLifeMenuCallback(CCObject* pSender)\n");
	modifyPlayerLifes(1);
}

void PlayScene::decrementLifeMenuCallback(CCObject* pSender)
{
	CCLOG("Entrando a: void PlayScene::decrementLifeMenuCallback(CCObject* pSender)\n");
	modifyPlayerLifes(-1);
}

void PlayScene::incrementBulletMenuCallback(CCObject* pSender)
{
	CCLOG("Entrando a: void PlayScene::incrementBulletMenuCallback(CCObject* pSender)\n");
	modifyPlayerBullets(1);
}

void PlayScene::decrementBulletMenuCallback(CCObject* pSender)
{
	CCLOG("Entrando a: void PlayScene::decrementBulletMenuCallback(CCObject* pSender)\n");
	modifyPlayerBullets(-1);
}

void PlayScene::activateMapCardModeCallback(CCObject* pSender)
{
	CCLOG("Entrando a: void PlayScene::activateMapCardModeCallback(CCObject* pSender)\n");
	if (fase[0]) PUTMAPCARD = true;
}

void PlayScene::redDiceCallback()
{
	CCLOG("Entrando a: void PlayScene::redDiceCallback()\n");
	int result;

	CCArray * actions = CCArray::createWithCapacity(7);
	CCFiniteTimeAction * single_action;

	single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::setMoveRedDice));
	actions->addObject(single_action);

	actions->addObject(CCDelayTime::create(1.5)); // 1.5 seg

	single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::setStaticRedDice));
	actions->addObject(single_action);

	// desactivar el wait
	single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::activateTouch), NULL);
	actions->addObject(single_action);

	if ( !HAY_BATALLA )
	{
		CCLOG ("No hay batalla\n");
		 result = GameState.rollZombieDice(); // [0,5]

		// Cambiar la subfase
		single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::changeSubPhase), NULL);
		actions->addObject(single_action);

		NUM_OF_ZOMBIES_TO_MOVE = result + 1;
	} else {
		result = GameState.rollFightDice(); // [0,5]

		if ( result < 3 ) {

			if (GameState.getCurrentPlayerLife() == 0 && (result + GameState.getCurrentPlayerBullet()) < 3){
				single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::loseBattleAndDie));
				actions->addObject(single_action);

			} else {
				// Jugador debe tomar una decision: perder vidas o balas
				single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::continueBattle));
				actions->addObject(single_action);
			}

		} else {
			// Jugador ha ganado
			single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::winBattle), NULL);
			actions->addObject(single_action);
		}
	}

	CCSequence *sq = CCSequence::create(actions);

	redDices[lastRedDiceResult]->setVisible(false);
	lastRedDiceResult = result;
	redDices[lastRedDiceResult]->runAction( sq );
}

void PlayScene::setMoveRedDice(CCNode* node)
{
	CCLOG("Entrando a: void PlayScene::setMoveRedDice(CCNode* node)\n");
	CCSpriteFrameCache* cache = CCSpriteFrameCache::sharedSpriteFrameCache();

	animatedSprite = CCSprite::createWithSpriteFrameName( "movedicered_1.png" );
	animatedSprite->setPosition( ccp( redDices[0]->getPosition().x, redDices[0]->getPosition().y) );

	spritebatch = CCSpriteBatchNode::create("sprite/movedicered.png");
	spritebatch->addChild(animatedSprite);

	_stayLayer->addChild(spritebatch);

	CCArray* animFrames = CCArray::createWithCapacity(16);

	char str[70] = {0};
	for(int i = 1; i < 16; i++)
	{
		sprintf(str, "movedicered_%d.png", i);
		CCSpriteFrame* frame = cache->spriteFrameByName( str );
		animFrames->addObject(frame);
	}

	CCAnimation* animation = CCAnimation::createWithSpriteFrames(animFrames, 0.1f);
	animatedSprite->runAction( CCRepeatForever::create( CCAnimate::create(animation) ) );

	animatedSprite->setFlipX(false);
	animatedSprite->setFlipY(false);
}

void PlayScene::setStaticRedDice(CCNode* node)
{
	CCLOG("Entrando a: void PlayScene::setStaticRedDice(CCNode* node) \n");
	if (spritebatch != NULL) {
		_stayLayer->removeChild(spritebatch);
		spritebatch = NULL;
		animatedSprite = NULL;
	}

	redDices[lastRedDiceResult]->setVisible(true);

	if ( !HAY_BATALLA )
		showContinueButton(NULL, NULL);
}

void PlayScene::blueDiceCallback()
{
	CCLOG("Entrando a: void PlayScene::blueDiceCallback() \n");
	int result = GameState.rollPlayerDice();
	CCArray * actions = CCArray::createWithCapacity(4);
	CCFiniteTimeAction * single_action;
	single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::setMoveBlueDice));
	actions->addObject(single_action);

	actions->addObject(CCDelayTime::create(1.5)); // 1.5 seg

	single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::setStaticBlueDice));
	actions->addObject(single_action);

	// desactivar el wait ( **OJO**)
	single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::activateTouch), NULL);
	actions->addObject(single_action);

	// Cambiar la subfase
	single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::changeSubPhase), NULL);
	actions->addObject(single_action);

	CCSequence *sq = CCSequence::create(actions);
	blueDices[lastBlueDiceResult]->setVisible(false);
	lastBlueDiceResult = result;
	blueDices[lastBlueDiceResult]->runAction( sq );
}

void PlayScene::setMoveBlueDice(CCNode* node)
{
	CCLOG("Entrando a: void PlayScene::setMoveBlueDice(CCNode* node) \n");
	CCSpriteFrameCache* cache = CCSpriteFrameCache::sharedSpriteFrameCache();

	animatedSprite = CCSprite::createWithSpriteFrameName( "movediceblue_1.png" );
	animatedSprite->setPosition( ccp( blueDices[0]->getPosition().x, blueDices[0]->getPosition().y) );

	spritebatch = CCSpriteBatchNode::create("sprite/movediceblue.png");
	spritebatch->addChild(animatedSprite);

	_stayLayer->addChild(spritebatch);

	CCArray* animFrames = CCArray::createWithCapacity(16);

	char str[70] = {0};
	for(int i = 1; i < 16; i++)
	{
		sprintf(str, "movediceblue_%d.png", i);
		CCSpriteFrame* frame = cache->spriteFrameByName( str );
		animFrames->addObject(frame);
	}

	CCAnimation* animation = CCAnimation::createWithSpriteFrames(animFrames, 0.1f);
	animatedSprite->runAction( CCRepeatForever::create( CCAnimate::create(animation) ) );

	animatedSprite->setFlipX(false);
	animatedSprite->setFlipY(false);
}

void PlayScene::setStaticBlueDice(CCNode* node)
{
	CCLOG("Entrando a: void PlayScene::setStaticBlueDice(CCNode* node)  \n");
	if (spritebatch != NULL) {
		_stayLayer->removeChild(spritebatch);
		spritebatch = NULL;
		animatedSprite = NULL;
	}

	blueDices[lastBlueDiceResult]->setVisible(true);

	CCPoint point;
	possibleMoves = GameState.getPossibleMoves().first;

	// Se muestran las posibilidades en la interfaz
	addPlayerBox();

	showContinueButton(NULL, NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * Funciones para colocar elementos en los layers  *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

void PlayScene::setMenuBackMenu(CCLayer *mLayer)
{
	CCLOG("Entrando a: void PlayScene::setMenuBackMenu(CCLayer *mLayer)\n");
	CCMenuItemImage * itemImage = CCMenuItemImage::create("Menu.png","Menu.png",this,menu_selector(PlayScene::mainMenuCallback));

	itemImage->setPosition(ccp(VisibleRect::rightBottom().x - itemImage->getContentSize().width/2,
                               VisibleRect::rightBottom().y + itemImage->getContentSize().height/2));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(itemImage, NULL);
    pMenu->setPosition(CCPointZero);
    mLayer->addChild(pMenu, 1);
}

void PlayScene::setOptionsLifeMenu(CCLayer *mLayer)
{
	CCLOG("Entrando a: void PlayScene::setOptionsLifeMenu(CCLayer *mLayer)\n");
	// +++++++++++++++++++++++++++++++++
	CCLabelTTF* label = CCLabelTTF::create("+", "Arial", FONT_SIZE + 10);
	CCMenuItemLabel* itemLabel = CCMenuItemLabel::create(label,this,menu_selector(PlayScene::incrementLifeMenuCallback));

	itemLabel->setPosition(ccp(VisibleRect::rightBottom().x - itemLabel->getContentSize().width/2 -30,
                               VisibleRect::rightBottom().y + itemLabel->getContentSize().height/2 +50));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(itemLabel, NULL);
    pMenu->setPosition(CCPointZero);
    mLayer->addChild(pMenu);

    // ----------------------------------
	label = CCLabelTTF::create("-", "Arial", FONT_SIZE +10);
	itemLabel = CCMenuItemLabel::create(label,this,menu_selector(PlayScene::decrementLifeMenuCallback));

	itemLabel->setPosition(ccp(VisibleRect::rightBottom().x - itemLabel->getContentSize().width/2 -10,
                               VisibleRect::rightBottom().y + itemLabel->getContentSize().height/2 +50));

    // create menu, it's an autorelease object
    pMenu = CCMenu::create(itemLabel, NULL);
    pMenu->setPosition(CCPointZero);
    mLayer->addChild(pMenu);
}

void PlayScene::setOptionsBulletMenu(CCLayer *mLayer)
{
	CCLOG("Entrando a: void PlayScene::setOptionsBulletMenu(CCLayer *mLayer)\n");
	// +++++++++++++++++++++++++++++++++
	CCLabelTTF* label = CCLabelTTF::create("+", "Arial", FONT_SIZE + 10);
	CCMenuItemLabel* itemLabel = CCMenuItemLabel::create(label,this,menu_selector(PlayScene::incrementBulletMenuCallback));

	itemLabel->setPosition(ccp(VisibleRect::rightBottom().x - itemLabel->getContentSize().width/2 -30,
                               VisibleRect::rightBottom().y + itemLabel->getContentSize().height/2 +100));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(itemLabel, NULL);
    pMenu->setPosition(CCPointZero);
    mLayer->addChild(pMenu);

    // ----------------------------------
	label = CCLabelTTF::create("-", "Arial", FONT_SIZE +10);
	itemLabel = CCMenuItemLabel::create(label,this,menu_selector(PlayScene::decrementBulletMenuCallback));

	itemLabel->setPosition(ccp(VisibleRect::rightBottom().x - itemLabel->getContentSize().width/2 -10,
                               VisibleRect::rightBottom().y + itemLabel->getContentSize().height/2 +100));

    // create menu, it's an autorelease object
    pMenu = CCMenu::create(itemLabel, NULL);
    pMenu->setPosition(CCPointZero);
    mLayer->addChild(pMenu);
}

void PlayScene::setDices(CCLayer * layer)
{
	CCLOG("Entrando a: void PlayScene::setDices(CCLayer * layer)\n");
	CCSprite* icon;

	char str[70] = {0};
	for (int i = 1; i <= 6; i++){

		sprintf(str, "sprite/diceblue_%d.png", i);
		icon = CCSprite::create(str);
		icon->setPosition(ccp(VisibleRect::leftBottom().x + icon->getContentSize().width/2 + 5,
							  VisibleRect::leftBottom().y + icon->getContentSize().height/2 + 5));
		icon->setVisible(false);
		layer->addChild(icon);
		blueDices.push_back(icon);

		sprintf(str, "sprite/dicered_%d.png", i);
		icon = CCSprite::create(str);
		icon->setPosition(ccp(VisibleRect::leftBottom().x + 3*icon->getContentSize().width/2 + 10 ,
							  VisibleRect::leftBottom().y + icon->getContentSize().height/2 + 5));
		icon->setVisible(false);
		layer->addChild(icon);
		redDices.push_back(icon);
	}

}

void PlayScene::setMap(CCLayer *mLayer)
{
	CCLOG("Entrando a: void PlayScene::setMap(CCLayer *mLayer)\n");
	CCSprite* mapCardSprite;
	CCPoint zeroPos = mapCardMatrixToAxis(0,0), currPos;

	mapCardSprite = CCSprite::create(mapPath[0].c_str()); // Plaza Central
	mapCardSprite->setPosition(mapCardMatrixToAxis(MAX_MAP_DIM/2, MAX_MAP_DIM/2));
	mLayer->addChild(mapCardSprite);
}

void PlayScene::setInterface()
{
	CCLOG("Entrando a: void PlayScene::setInterface()\n");
	CCSprite*   icon;
	CCLabelTTF* label;
	int off_y = 5;
	int off_x = 8;

	icon = CCSprite::create("lifeIcon.png");
	icon->setPosition(ccp(VisibleRect::leftTop().x + off_x + icon->getContentSize().width/2,
						  VisibleRect::leftTop().y - icon->getContentSize().height/2 - off_y));
	icon->setTag(LIFE_ICON_TAG);


	label = CCLabelTTF::create("x 0", "Arial", FONT_SIZE);
	label->setTag(LIFES_LABEL_TAG);
	label->setPosition(ccp(VisibleRect::leftTop().x + icon->getContentSize().width + off_x + 7 + label->getContentSize().width/2,
						   VisibleRect::leftTop().y - icon->getContentSize().height/2 - off_y));

	off_y += icon->getContentSize().height + 5;

	_stayLayer->addChild(icon, 2);
	_stayLayer->addChild(label, 2);

	icon = CCSprite::create("bulletIcon.png");
	icon->setPosition(ccp(VisibleRect::leftTop().x + off_x + icon->getContentSize().width/2,
						  VisibleRect::leftTop().y - icon->getContentSize().height/2 - off_y));
	icon->setTag(BULLET_ICON_TAG);

	label = CCLabelTTF::create("x 0", "Arial", FONT_SIZE);
	label->setTag(BULLETS_LABEL_TAG);
	label->setPosition(ccp(VisibleRect::leftTop().x + icon->getContentSize().width + 7 + off_x + label->getContentSize().width/2,
						   VisibleRect::leftTop().y - icon->getContentSize().height/2 - off_y));

	_stayLayer->addChild(icon, 2);
	_stayLayer->addChild(label, 2);

	off_x = 8;
	off_y = 5;

	icon = CCSprite::create("zombie.png");
	icon->setPosition(ccp(VisibleRect::rightTop().x - off_x - icon->getContentSize().width/2,
						  VisibleRect::rightTop().y - icon->getContentSize().height/2 - off_y));
	icon->setTag(ZOMBIE_ICON_TAG);

	off_x += icon->getContentSize().width;

	label = CCLabelTTF::create("0 x", "Arial", FONT_SIZE);
	label->setTag(ZOMBIES_LABEL_TAG);
	label->setPosition(ccp(VisibleRect::rightTop().x - off_x - 5 - label->getContentSize().width/2,
						   VisibleRect::rightTop().y - icon->getContentSize().height/2 - off_y));

	_stayLayer->addChild(icon, 2);
	_stayLayer->addChild(label, 2);

	label = CCLabelTTF::create("Player 1", "Arial", FONT_SIZE + 8);
	label->setTag(TITLE_LABEL_TAG);
	label->setPosition(ccp(VisibleRect::top().x,
						   VisibleRect::top().y - label->getContentSize().height/2));

	_stayLayer->addChild(label, 2);
	off_y = label->getContentSize().height + 5;

	label = CCLabelTTF::create(name[currFase][currSubFase].c_str(), "Arial", FONT_SIZE);
	label->setTag(SUBTITLE_LABEL_TAG);
	label->setPosition(ccp(VisibleRect::top().x,
						   VisibleRect::top().y - off_y));

	_stayLayer->addChild(label, 2);


	icon = CCSprite::create("lifeIcon.png");
	icon->setPosition(ccp(VisibleRect::bottom().x - icon->getContentSize().width,
						  VisibleRect::bottom().y + icon->getContentSize().height/2 + 5));
	icon->setTag(QUESTION_LIFE_ICON_TAG);
	icon->setVisible(false);
	_stayLayer->addChild(icon, 2);

	off_y = icon->getContentSize().height + 15;

	icon = CCSprite::create("bulletIcon.png");
	icon->setPosition(ccp(VisibleRect::bottom().x + icon->getContentSize().width,
						  VisibleRect::bottom().y + icon->getContentSize().height/2 + 5));
	icon->setTag(QUESTION_BULLET_ICON_TAG);
	icon->setVisible(false);
	_stayLayer->addChild(icon, 2);


	label = CCLabelTTF::create("¿Qué desea utilizar?", "Arial", FONT_SIZE);
	label->setTag(QUESTION_LABEL_TAG);
	label->setPosition(ccp(VisibleRect::bottom().x,
						   VisibleRect::bottom().y + off_y));
	label->setVisible(false);
	_stayLayer->addChild(label, 2);

	// Boton de Skip movimiento de zombie o player:

	CCMenuItemImage * itemImage = CCMenuItemImage::create("Continuar.png","Continuar.png",this,menu_selector(PlayScene::skipMenuCallback));

	itemImage->setPosition(ccp(VisibleRect::right().x - itemImage->getContentSize().width/2 - 5,
			                   VisibleRect::right().y - itemImage->getContentSize().height/2));

    CCMenu* pMenu = CCMenu::create(itemImage, NULL);
    pMenu->setPosition(CCPointZero);
    pMenu->setVisible(false);
    pMenu->setTag(CONTINUAR_LABEL_TAG);
    _stayLayer->addChild(pMenu, 2);
}

void PlayScene::setPlayerInfo()
{
	CCLOG("Entrando a: void PlayScene::setPlayerInfo()\n");
	CCLabelTTF *label = (CCLabelTTF *) _stayLayer->getChildByTag(TITLE_LABEL_TAG);
	char info[45];
	sprintf(info, "Jugador %d", GameState.getCurrentPlayer() + 1);
	label->setString(info);

	modifyScreenLifes();
	modifyScreenBullets();
	modifyScreenZombies();
}

void PlayScene::modifyPlayerLifes(int num)
{
	CCLOG("Entrando a: void PlayScene::modifyPlayerLifes(int num)\n");
	if (num == 1) GameState.addLife();
	else if (num == -1) GameState.decreaseLife();

	modifyScreenLifes();
}

void PlayScene::modifyPlayerBullets(int num)
{
	CCLOG("Entrando a: void PlayScene::modifyPlayerBullets(int num)\n");
	if (num == 1) GameState.addBullet();
	else if (num < 0) GameState.decreaseBullet(-num);

	modifyScreenBullets();
}

void PlayScene::modifyPlayerZombies()
{
	CCLOG("Entrando a: void PlayScene::modifyPlayerZombies()\n");
	GameState.addZombie();

	modifyScreenZombies();
}

void PlayScene::modifyScreenLifes()
{
	CCLOG("Entrando a: void PlayScene::modifyScreenLifes()\n");
	CCLabelTTF *label = (CCLabelTTF *) _stayLayer->getChildByTag(LIFES_LABEL_TAG);
	char info[10];
	sprintf(info, "x %d", GameState.getCurrentPlayerLife());

	label->setString(info);
}

void PlayScene::modifyScreenBullets()
{
	CCLOG("Entrando a: void PlayScene::modifyScreenBullets()\n");
	CCLabelTTF *label = (CCLabelTTF *) _stayLayer->getChildByTag(BULLETS_LABEL_TAG);
	char info[10];
	sprintf(info, "x %d", GameState.getCurrentPlayerBullet());

	label->setString(info);
}

void PlayScene::modifyScreenZombies()
{
	CCLOG("Entrando a: void PlayScene::modifyScreenZombies()\n");
	CCLabelTTF *label = (CCLabelTTF *) _stayLayer->getChildByTag(ZOMBIES_LABEL_TAG);
	char info[10];
	sprintf(info, "%d x", GameState.getCurrentPlayerZombie());

	label->setString(info);
}

void PlayScene::setSprites(CCLayer* mLayer)
{
	CCLOG("Entrando a: void PlayScene::setSprites(CCLayer* mLayer)\n");
	CCPoint center = mapCardMatrixToAxis(MAX_MAP_DIM/2, MAX_MAP_DIM/2);

	center.y += (mSprite[0]->getContentSize().height / 2) + 5;

	for(int i = 0; i < NUM_OF_PLAYER; i++){
		if (i == 0)
			center.x += (mSprite[i]->getContentSize().width / 2) + 7;
		else
			center.x -= (mSprite[i]->getContentSize().width / 2) + 7;

		mSprite[i]->setPosition(center);
		mLayer->addChild(mSprite[i], 5);

		if (i == 0)
			center.x -= (mSprite[i]->getContentSize().width / 2) + 7;
		else
			center.x += (mSprite[i]->getContentSize().width / 2) + 7;
	}
}

void PlayScene::setMoveSprite(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::setMoveSprite(CCNode* sender, void* data) \n");
	CCSpriteFrameCache* cache = CCSpriteFrameCache::sharedSpriteFrameCache();
	Event *d =  (Event*) data;
	string name;

	if (d->orientation == 'H')
		if (d->movement > 0)
			name = NAME_PLAYER_SPRITE_ANIMATION[GameState.getCurrentPlayer()][RIGHT];
		else
			name = NAME_PLAYER_SPRITE_ANIMATION[GameState.getCurrentPlayer()][LEFT];
	else
		if (d->movement > 0)
			name = NAME_PLAYER_SPRITE_ANIMATION[GameState.getCurrentPlayer()][UP];
		else
			name = NAME_PLAYER_SPRITE_ANIMATION[GameState.getCurrentPlayer()][DOWN];

	animatedSprite = CCSprite::createWithSpriteFrameName( (name +"_1.png").c_str() );
	animatedSprite->setPosition( ccp( mSprite[GameState.getCurrentPlayer()]->getPositionX(), mSprite[GameState.getCurrentPlayer()]->getPositionY()) );
	mSprite[GameState.getCurrentPlayer()]->setVisible(false);

	spritebatch = CCSpriteBatchNode::create( ("sprite/" +name +".png" ).c_str());
	spritebatch->addChild(animatedSprite);

	_moveLayer->addChild(spritebatch, 5);

	CCArray* animFrames = CCArray::createWithCapacity(5);

	char str[250] = {0};
	for(int i = 1; i < 5; i++)
	{
		sprintf(str, "%s_%d.png", name.c_str(), i);
		CCSpriteFrame* frame = cache->spriteFrameByName( str );
		animFrames->addObject(frame);
	}

	CCAnimation* animation = CCAnimation::createWithSpriteFrames(animFrames, 0.3f);
	animatedSprite->runAction( CCRepeatForever::create( CCAnimate::create(animation) ) );

	animatedSprite->setFlipX(false);
	animatedSprite->setFlipY(false);
}

void PlayScene::animateSprite(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::animateSprite(CCNode* sender, void* data)  \n");
	CCFiniteTimeAction* actionMove;
	Event *d =  (Event*) data;
	float offX = 0, offY = 0;

	if (d->orientation== 'H')
		offX += d->movement;
	else
		offY += d->movement;

	actionMove = CCMoveTo::create( ( abs(d->movement)) / VELOCITY_SPRITE_MOVEMENT,
									ccp(mSprite[GameState.getCurrentPlayer()]->getPositionX() + offX, mSprite[GameState.getCurrentPlayer()]->getPositionY() +offY));


	mSprite[GameState.getCurrentPlayer()]->setPosition(ccp(mSprite[GameState.getCurrentPlayer()]->getPositionX() + offX, mSprite[GameState.getCurrentPlayer()]->getPositionY()+ offY));

	animatedSprite->runAction(actionMove);
}

void PlayScene::setStaticSprite(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::setStaticSprite(CCNode* sender, void* data) \n");
	Event *d =  (Event*) data;

	GameState.movePlayerTo(d->end_point);

	if (spritebatch != NULL ) {
		_moveLayer->removeChild(spritebatch);
		spritebatch = NULL;
		animatedSprite = NULL;
	}

	mSprite[GameState.getCurrentPlayer()]->setVisible(true);

	//OJO no seguro
	if (!GameState.queryZombie()) {
		checkLifeAndBullet(NULL, NULL);

		// Chequear fin de juego
		if (GameState.currentPlayerOverHeliport()) {
			gameOver();
		}
	}
}








void PlayScene::setMoveZombieSprite(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::setMoveZombieSprite(CCNode* sender, void* data)  \n");

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/zombie_walk.wav");

	CCSpriteFrameCache* cache = CCSpriteFrameCache::sharedSpriteFrameCache();
	Event *d =  (Event*) data;
	string name;

	if (d->orientation == 'H')
		if (d->movement > 0)
			name = NAME_ZOMBIE_SPRITE_ANIMATION[RIGHT];
		else
			name = NAME_ZOMBIE_SPRITE_ANIMATION[LEFT];
	else
		if (d->movement > 0)
			name = NAME_ZOMBIE_SPRITE_ANIMATION[UP];
		else
			name = NAME_ZOMBIE_SPRITE_ANIMATION[DOWN];

	animatedSprite = CCSprite::createWithSpriteFrameName( (name +"_1.png").c_str() );
	animatedSprite->setPosition( prevZombieLocation );
	zombies[zombieSpriteToMove].sprite->setVisible(false);

	// Posicion previa del zombie no se necesita mas
	prevZombieLocation.x = prevZombieLocation.y = -1;

	spritebatch = CCSpriteBatchNode::create( ("sprite/" +name +".png" ).c_str());
	spritebatch->addChild(animatedSprite);

	_moveLayer->addChild(spritebatch, 5);

	CCArray* animFrames = CCArray::createWithCapacity(5);

	char str[250] = {0};
	int i;
	for(i = 1; i < 5; i++)
	{
		sprintf(str, "%s_%d.png", name.c_str(), i);
		CCSpriteFrame* frame = cache->spriteFrameByName( str );
		animFrames->addObject(frame);
	}

	CCAnimation* animation = CCAnimation::createWithSpriteFrames(animFrames, 0.3f);
	animatedSprite->runAction( CCRepeatForever::create( CCAnimate::create(animation) ) );

	animatedSprite->setFlipX(false);
	animatedSprite->setFlipY(false);
}

void PlayScene::animateZombieSprite(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::animateZombieSprite(CCNode* sender, void* data) \n");
	CCFiniteTimeAction* actionMove;
	Event *d =  (Event*) data;
	float offX = 0, offY = 0;

	if (d->orientation == 'H')
		offX += d->movement;
	else
		offY += d->movement;

	actionMove = CCMoveTo::create( ( abs(d->movement)) / VELOCITY_SPRITE_MOVEMENT,
									ccp(zombies[zombieSpriteToMove].sprite->getPositionX() + offX, zombies[zombieSpriteToMove].sprite->getPositionY() +offY));

	zombies[zombieSpriteToMove].sprite->setPosition(ccp(zombies[zombieSpriteToMove].sprite->getPositionX() + offX, zombies[zombieSpriteToMove].sprite->getPositionY()+ offY));

	animatedSprite->runAction(actionMove);
}

void PlayScene::setStaticZombieSprite(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::setStaticZombieSprite(CCNode* sender, void* data) \n");
	if (spritebatch != NULL) {
		_moveLayer->removeChild(spritebatch);
		spritebatch = NULL;
		animatedSprite = NULL;
	}

	zombies[zombieSpriteToMove].sprite->setVisible(true);
}





/*
 * Verifica si la posicion p, es valida para la carta
 * con la rotacion actual
 */

bool PlayScene::isAllowed(position p)
{
	CCLOG("Entrando a: bool PlayScene::isAllowed(position p) \n");
	for (int i = 0; i < allowedPositions[currCardRotation].size(); i++) {
		if (allowedPositions[currCardRotation][i] == p)
			return true;
	}

	return false;
}


bool PlayScene::putMapCard(CCPoint location)
{
	CCLOG("Entrando a: bool PlayScene::putMapCard(CCPoint location, int id)\n");
	position p = position(location.x, location.y);

	if (!isAllowed(p)) {
		if (GameState.isCurrentPlayerMachine()) return true;

		return false;
	}

	CCSprite* sprite;
	int fila, columna;

	// Se rota la carta y se notifica al modelo
	mapCard &current_mapcard = GameState.getLastMapCard();

	for (int i = 0; i < currCardRotation; i++)
		current_mapcard.rotateR();

	GameState.putCardMap(current_mapcard, p);

	// Se cambia el sistema de referencia
	p.t();

	// Se coloca la carta de mapa en la interfaz
	sprite = CCSprite::create(GameState.getLastMapCard().getPath().c_str());
	sprite->setPosition(mapCardMatrixToAxis(p.x / 3, p.y / 3));
	sprite->setRotation(currCardRotation * 90);
	_moveLayer->addChild(sprite, 1);

	// Se eliminan los box de las posibles posiciones de un mapcard
	removeMapCardBox();

	float bv_offset_x, bv_offset_y, z_offset_y;

	//Zombie
	sprite = CCSprite::create("sprite/zombie.png");
	z_offset_y = - sprite->getContentSize().height/4;

	// vida y bala
	sprite = CCSprite::create("lifeIcon.png");
	bv_offset_x = -PIXELS_TILE/2 + sprite->getContentSize().width/2 + 5;
	bv_offset_y = -PIXELS_TILE/2 + sprite->getContentSize().height/2 + 5;

	// Obteniendo la fila y columna del mapCard nuevo
	fila = p.x / 3;
	columna = p.y / 3;

	Element element;
	element.mapCard_i = fila;
	element.mapCard_j = columna;


	// Se colocan los elementos que tiene (zombies, vidas, balas):
	CCPoint actualLocation;
	for (int i = 0; i < MAPNUMPOS; i++){
		element.tile_i = i;
		for (int j = 0; j < MAPNUMPOS; j++){
			actualLocation = tileMatrixToAxis(fila, columna, i, j);
			tile currTile = GameState.getLastMapCard().getTile(i-1, j-1);
			element.tile_j = j;
			if (currTile.hasZombie()) {
				element.sprite = CCSprite::create("sprite/zombie.png");
				element.sprite->setPosition(ccp(actualLocation.x, actualLocation.y + z_offset_y));
				_moveLayer->addChild(element.sprite,4);
				zombies.push_back(element);
			}

			if (currTile.hasBullet()) {
				element.sprite = CCSprite::create("bulletIcon.png");
				element.sprite->setPosition(ccp(actualLocation.x + bv_offset_x, actualLocation.y + bv_offset_y));
				_moveLayer->addChild(element.sprite,2);
				bullets.push_back(element);
			}

			if (currTile.hasLife()) {
				element.sprite = CCSprite::create("lifeIcon.png");
				element.sprite->setPosition(ccp(actualLocation.x + bv_offset_x, actualLocation.y + bv_offset_y));
				_moveLayer->addChild(element.sprite,2);
				lifes.push_back(element);
			}
		}
	}

	PUTMAPCARD = false;

	return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * *
 *     Funciones para llevar a cabo acciones       *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

position PlayScene::relativeTileToAbsoluteTile(int mx, int my, int tx, int ty)
{
  int x = mx * 3 - MAPMID - (1 - tx);
  int y = my * 3 - MAPMID - (1 - ty);

  return position(x,y);
}

CCPoint PlayScene::axisToMapCardMatrix(float x, float y)
{
	float aux_x = abs( PIXELS_MAP_CARD - x );
	float aux_y = abs( PIXELS_MAP_CARD + MAX_MAP_DIM * PIXELS_MAP_CARD - y );

	return ccp(aux_y/PIXELS_MAP_CARD, aux_x/PIXELS_MAP_CARD);
}

CCPoint PlayScene::axisToTileMatrix(float x, float y)
{
	float aux_x = abs( PIXELS_MAP_CARD - x );
	float aux_y = abs( PIXELS_MAP_CARD + MAX_MAP_DIM * PIXELS_MAP_CARD - y );

	return ccp( (int)(aux_y/PIXELS_TILE) % 3, (int)(aux_x/PIXELS_TILE) % 3);
}

CCPoint PlayScene::mapCardMatrixToAxis(int i, int j)
{
	float origen_x = PIXELS_MAP_CARD + PIXELS_MAP_CARD/2;
	float origen_y = PIXELS_MAP_CARD + MAX_MAP_DIM* PIXELS_MAP_CARD - PIXELS_MAP_CARD/2;

	return ccp(origen_x + j * PIXELS_MAP_CARD, origen_y + (-i * PIXELS_MAP_CARD) );
}

CCPoint PlayScene::tileMatrixToAxis(int i_mapCard, int j_mapCard, int i, int j)
{
	CCPoint center = mapCardMatrixToAxis(i_mapCard,j_mapCard);

	return ccp(center.x + PIXELS_TILE * (j-1), center.y - PIXELS_TILE * (i-1) );
}

Event PlayScene::getEventZombieMove(CCPoint prev, CCPoint next)
{
	CCLOG("Entrando a: Event PlayScene::getEventZombieMove(CCPoint prev, CCPoint next)\n");
	int prev_x = abs( PIXELS_MAP_CARD - prev.x) / PIXELS_TILE;
	int prev_y = abs( (PIXELS_MAP_CARD + MAX_MAP_DIM * PIXELS_MAP_CARD) - prev.y) / PIXELS_TILE;

	int next_x = abs( PIXELS_MAP_CARD - next.x) / PIXELS_TILE;
	int next_y = abs( (PIXELS_MAP_CARD + MAX_MAP_DIM * PIXELS_MAP_CARD) - next.y) / PIXELS_TILE;

	Event evento;
	CCPoint point = axisToMapCardMatrix(next.x, next.y);
	point = tileMatrixToAxis((int)point.x, (int)point.y, next_y % 3, next_x % 3);

	if ( abs(next_x - prev_x) == 1 && abs(next_y - prev_y) == 0 ){
		evento.orientation = 'H';
		evento.movement = point.x - prev.x;
		return evento;
	}

	if ( abs(next_x - prev_x) == 0 && abs(next_y - prev_y) == 1 ){
		evento.orientation = 'V';
		evento.movement = point.y - prev.y;
		return evento;
	}

	evento.orientation = '-';
	return evento;
}

void PlayScene::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)
{
	CCLOG("Entrando a: void PlayScene::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent)\n");
	CCTouch *touch = (CCTouch*) (*pTouches->begin());
	CCPoint pto = _moveLayer->convertTouchToNodeSpace(touch);
	CCLOG ("%f %f", pto.x, pto.y);

	_moveLayer->ccTouchesBegan(pTouches, pEvent);
	touches++;
	if (touches == 1) singleTouch = true;
	else singleTouch = false;
}

void PlayScene::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)
{
	CCLOG("Entrando a: void PlayScene::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)\n");
	_moveLayer->ccTouchesMoved(pTouches, pEvent);

    if (touches != 1)
    	return;

	CCTouch *touch = (CCTouch*) (*pTouches->begin());
	CCPoint curTouchPosition = CCDirector::sharedDirector()->convertToGL(touch->getLocationInView());
	CCPoint prevTouchPosition = CCDirector::sharedDirector()->convertToGL(touch->getPreviousLocationInView());

	touchDistance += ccpDistance(curTouchPosition, prevTouchPosition);
}

void PlayScene::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent)
{
	CCLOG("Entrando a: void PlayScene::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent)\n");
	_moveLayer->ccTouchesEnded(pTouches, pEvent);

	if (touches == 1 && singleTouch && touchDistance <= EPSILON_DISTANCE && !WAIT) {

    	CCTouch * touch = (CCTouch *) (*pTouches->begin());
    	CCPoint ptoConvertido = _moveLayer->convertTouchToNodeSpace(touch);

    	if (!GameState.isCurrentPlayerMachine())
			if      (currFase == 0) firstPhase(touch->getLocation(), ptoConvertido);
			else if (currFase == 1) secondPhase(touch->getLocation(), ptoConvertido);
			else if (currFase == 2) thirdPhase(touch->getLocation(), ptoConvertido);

	}

    touchDistance = 0.0;
    if (touches > 0) touches--;
}

void PlayScene::ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent)
{
	CCLOG("Entrando a: void PlayScene::ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent)\n");
	_moveLayer->ccTouchesCancelled(pTouches, pEvent);

	if (touches > 0) touches--;
	if (touches == 0) {
		touchDistance = 0.0;
	}
}

void PlayScene::pointToEvent(position p)
{
	CCLOG("Entrando a: void PlayScene::pointToEvent(position p)\n");
	// Obtenemos el "touch" del usuario y lo enviamos al Modelo para que nos de el
	// arreglo con los eventos. provisional

	vector<position> road = GameState.queryMovePlayerTo(p);

	events.clear();
	Event e;

	for (int i = 1; i < road.size() && i < 20; i++){

		if ( road[i-1].x != road[i].x ) {
			e.orientation = 'V';
			if (road[i-1].x > road[i].x)
				e.movement = PIXELS_TILE;
			else
				e.movement = -PIXELS_TILE;
		} else {
			e.orientation = 'H';
			if (road[i-1].y > road[i].y)
				e.movement = -PIXELS_TILE;
			else
				e.movement = PIXELS_TILE;
		}

		e.end_point = road[i];
		events.push_back(e);
	}
}

void PlayScene::activateTouch(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::activateTouch(CCNode* sender, void* data)\n");
	WAIT = false;
}

void PlayScene::changePhase(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::changePhase(CCNode* sender, void* data)\n");
	currFase = (currFase + 1) % NUM_FASES;
	currSubFase = 0;


	if (currFase == 0) {
		// Cambio de Jugador
		GameState.nextTurn();
		setPlayerInfo();

		// Retornar el sprite de la carta a su forma original
		currCardRotation = 0;

		hideThirdPhase();

		// Pila de cartas vacia
		if (GameState.mapStackEmpty())
			currFase = (currFase + 1) % NUM_FASES;

		showFirstPhase();
	}

	// En este punto se tiene certeza de la fase que sigue
	msg = name[currFase][currSubFase];
	putSubtitleInformation(NULL,NULL);

	if (currFase == 1) {
		possibleMoves.clear();
		LANZODADOAZUL = false; // provisional
		HAY_BATALLA = false;
		HAY_PREGUNTA = false;
		hideFirstPhase();
		showSecondPhase();
		checkBattle(NULL,NULL);
	}

	if (currFase == 2) {
		LANZODADOROJO = false; // provisional
		hideSecondPhase();
		showThirdPhase();
	}

	// Si es la maquina se juega la fase automaticamente
	if (GameState.isCurrentPlayerMachine()) {
			if      (currFase == 0) firstPhase(ccp(-1, -1), ccp(-1, -1));
			else if (currFase == 2) thirdPhase(ccp(-1, -1), ccp(-1, -1));
	}

}

void PlayScene::changeSubPhase(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::changeSubPhase(CCNode* sender, void* data)\n");
	currSubFase++;
	CCLabelTTF * label = (CCLabelTTF *) _stayLayer->getChildByTag(SUBTITLE_LABEL_TAG);
	label->setString(name[currFase][currSubFase].c_str());

	// Si es la maquina se juega la fase automaticamente
	if (GameState.isCurrentPlayerMachine()) {
			if      (currFase == 0) firstPhase(ccp(-1, -1), ccp(-1, -1));
			else if (currFase == 1) secondPhase(ccp(-1, -1), ccp(-1, -1));
			else if (currFase == 2) thirdPhase(ccp(-1, -1), ccp(-1, -1));
	}
}

void PlayScene::putSubtitleInformation(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::putSubtitleInformation(CCNode* sender, void* data)\n");
	CCLabelTTF * label = (CCLabelTTF *) _stayLayer->getChildByTag(SUBTITLE_LABEL_TAG);
	label->setString(msg.c_str());
}

// FASES
void PlayScene::firstPhase(CCPoint pto, CCPoint ptoConvertido)
{
	CCLOG("Entrando a: void PlayScene::firstPhase(CCPoint pto, CCPoint ptoConvertido)\n");
	CCSprite * sprite = (CCSprite *) _stayLayer->getChildByTag(MAP_CARD_DECK);

	if (!PUTMAPCARD && (GameState.isCurrentPlayerMachine() || sprite->boundingBox().containsPoint(pto))) {
		WAIT = true;
		initFlipXCallback(NULL);
	}

	if (PUTMAPCARD)
	{
		// actualizar rotacion de la carta (variable) para
		// chequear correcta lista de posibles posiciones
		sprite = (CCSprite *) _stayLayer->getChildByTag(ROTATE_ICON);
		if (sprite->boundingBox().containsPoint(pto)) {
			WAIT = true;
			rotate_mapCard();
		} else {

			CCPoint p; // Donde será colocada la carta

			if (!GameState.isCurrentPlayerMachine()) // Juega el usuario
			{
				CCPoint actualLocation = axisToMapCardMatrix(ptoConvertido.x, ptoConvertido.y);
				position pos = relativeTileToAbsoluteTile((int) actualLocation.x, (int) actualLocation.y, 1, 1);
				p = ccp(pos.x, pos.y);
			} else // Juega la maquina
			{
				decision d;
				pair<int, position> choise = d.putmapcard(GameState);
				currCardRotation = choise.first;
				p = ccp(choise.second.x, choise.second.y);
			}

			if (putMapCard(p))
				changePhase(NULL, NULL);
		}
	}
}

bool PlayScene::canMove(position p)
{
	CCLOG("Entrando a: bool PlayScene::canMove(position p) \n");

	for (int i = 0; i < possibleMoves.size(); i++) {
		if (possibleMoves[i] == p)
			return true;
	}

	return false;
}

bool PlayScene::canMoveZombie(position prev_p, position p)
{
	CCLOG("Entrando a: bool PlayScene::canMoveZombie(position prev_p, position p) \n");
	vector<position> posibles_moves = GameState.getPossibleZombieMoves(prev_p);

	for (int i = 0; i < posibles_moves.size(); i++) {
		if (posibles_moves[i] == p)
			return true;
	}

	return false;
}

void PlayScene::secondPhase(CCPoint pto, CCPoint ptoConvertido)
{
	CCLOG("Entrando a: void PlayScene::secondPhase(CCPoint pto, CCPoint ptoConvertido) \n");
	// El dado azul fue cliqueado
	if ( !HAY_PREGUNTA && !LANZODADOAZUL && !HAY_BATALLA  && (blueDices[0]->boundingBox().containsPoint(pto) || GameState.isCurrentPlayerMachine()))
	{
		// Se muestra el jugador que va a tomar turno
		showCurrPlayerBox();

		// desactiva touch
		WAIT = true;
		blueDiceCallback();

		LANZODADOAZUL = true;
		return;
	}

	if (!HAY_BATALLA && LANZODADOAZUL && !HAY_PREGUNTA)
	{
		CCPoint index_cardMap = axisToMapCardMatrix(ptoConvertido.x, ptoConvertido.y);
		CCPoint index_tile = axisToTileMatrix(ptoConvertido.x,ptoConvertido.y);
		position p;
		if (!GameState.isCurrentPlayerMachine()) // Juega el usuario
		{
			p = relativeTileToAbsoluteTile(index_cardMap.x, index_cardMap.y, index_tile.x,index_tile.y);
			if (!canMove(p)) return;
		} else // Juega la maquina
		{
			decision d;
			p = d.movement(GameState);
			if (p == GameState.getCurrentPlayerPosition()) {
				removePlayerBox();
				changePhase(NULL, NULL);
				return;
			}
		}

		// desactiva touch
		WAIT = true;
		possibleMoves.clear();

		CCPoint point = tileMatrixToAxis(index_cardMap.x, index_cardMap.y, (int)index_tile.x, (int)index_tile.y);

		if (GameState.getCurrentPlayer() == 0)
			point.x += (mSprite[GameState.getCurrentPlayer()]->getContentSize().width / 2);
		else
			point.x -= (mSprite[GameState.getCurrentPlayer()]->getContentSize().width / 2);

		point.y += (mSprite[0]->getContentSize().height / 2);

		pointToEvent(p);

		CCArray *actions = CCArray::createWithCapacity(4 * events.size() + 3);
		CCFiniteTimeAction* single_action;

		for (int i = 0; i < events.size(); i++) {
			single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::setMoveSprite), (void*) &events[i]);
			actions->addObject(single_action);

			single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::animateSprite), (void*) &events[i]);
			actions->addObject(single_action);

			actions->addObject(CCDelayTime::create(abs(events[i].movement) / VELOCITY_SPRITE_MOVEMENT));

			single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::setStaticSprite), (void*) &events[i]);
			actions->addObject(single_action);
		}

		// activar touch
		single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::activateTouch), NULL);
		actions->addObject(single_action);

		/*
		 * Quedamos en que el arreglo de eventos tendria eventos hasta encontrarse con un zombie para
		 * batallar o hasta llegar al punto final del movimiento (en caso de que no haya batalla)
		 */

		single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::checkBattle), NULL);
		actions->addObject(single_action);

		CCSequence *sq = CCSequence::create(actions);

		// Se oculta el boton de continuar
		_stayLayer->getChildByTag(CONTINUAR_LABEL_TAG)->setVisible(false);

		mSprite[GameState.getCurrentPlayer()]->runAction( sq );

		removePlayerBox();
	}

	if ( HAY_BATALLA && !HAY_PREGUNTA && (redDices[0]->boundingBox().containsPoint(pto) || GameState.isCurrentPlayerMachine()))
	{
		// desactiva touch
		WAIT = true;
		redDiceCallback();
		return;
	}

	if ( HAY_PREGUNTA ) {
		CCSprite * life, * bullet;
		life = (CCSprite * ) _stayLayer->getChildByTag(QUESTION_LIFE_ICON_TAG);
		bullet = (CCSprite * ) _stayLayer->getChildByTag(QUESTION_BULLET_ICON_TAG);

		decision d;
		bool isMachine = GameState.isCurrentPlayerMachine();
		bool selectLife = d.selectLife(GameState);
		bool onlyLife = GameState.getCurrentPlayerBullet() + lastRedDiceResult < 3;
		if ((life->boundingBox().containsPoint(pto) || (isMachine && selectLife) || onlyLife)
				&& GameState.getCurrentPlayerLife() > 0) {
			modifyPlayerLifes(-1);

			msg = "Lanza el dado rojo de nuevo";
			putSubtitleInformation(NULL, NULL);

			HAY_PREGUNTA = false;
			_stayLayer->getChildByTag(QUESTION_LABEL_TAG)->setVisible(false);
			_stayLayer->getChildByTag(QUESTION_LIFE_ICON_TAG)->setVisible(false);
			_stayLayer->getChildByTag(QUESTION_BULLET_ICON_TAG)->setVisible(false);

			if (GameState.isCurrentPlayerMachine()){
				secondPhase(ccp(-1, -1), ccp(-1,-1));
			}

		} else if ((bullet->boundingBox().containsPoint(pto) || (isMachine && !selectLife))
				&& GameState.getCurrentPlayerBullet() + lastRedDiceResult >= 3) {
			modifyPlayerBullets(lastRedDiceResult - 3);
			redDices[lastRedDiceResult]->setVisible(false);
			lastRedDiceResult = 3;
			redDices[lastRedDiceResult]->setVisible(true);

			winBattle(NULL, NULL);

			HAY_PREGUNTA = false;
			_stayLayer->getChildByTag(QUESTION_LABEL_TAG)->setVisible(false);
			_stayLayer->getChildByTag(QUESTION_LIFE_ICON_TAG)->setVisible(false);
			_stayLayer->getChildByTag(QUESTION_BULLET_ICON_TAG)->setVisible(false);
		}

	}
}

void PlayScene::showCurrPlayerBox() {
	// Muestra en la interfaz el player a mover:
	boxTileSize = CCSizeMake(PIXELS_TILE, PIXELS_TILE);
	boxTile = CCLayerColor::create(ccc4(185, 32, 32, 0));

	position p = GameState.getCurrentPlayerPosition();
	p.t();

	CCPoint point = tileMatrixToAxis(p.x/3, p.y/3, p.x%3, p.y%3);

	boxTile->setPosition(ccp(point.x-boxTileSize.width/2, point.y-boxTileSize.height/2));
	boxTile->setContentSize(boxTileSize);
	boxTile->setOpacity((GLubyte)(140));
	_moveLayer->addChild(boxTile, 1, SELECTED_PLAYER_TAG);
	// FIN: Muestra en la interfaz el player a mover
}

void PlayScene::addPlayerBox()
{
	// Muestra en la interfaz los posibles movimientos del player:
	CCPoint point;

	for (int i = 0; i < possibleMoves.size(); i++) {
		boxTile = CCLayerColor::create(ccc4(0, 119, 255, 0));

		possibleMoves[i].t();

		point = tileMatrixToAxis(possibleMoves[i].x/3, possibleMoves[i].y/3, possibleMoves[i].x%3, possibleMoves[i].y%3);

		possibleMoves[i].invT();

		boxTile->setPosition(ccp(point.x-boxTileSize.width/2, point.y-boxTileSize.height/2));
		boxTile->setContentSize(boxTileSize);
		boxTile->setOpacity((GLubyte)(140));

		boxTileAdded.push_back(boxTile);
		_moveLayer->addChild(boxTile, 1);
	}
	// FIN: Muestra en la interfaz los posibles movimientos del player
}

void PlayScene::removePlayerBox()
{
	// Se remueven los child que muestran al usuario el player seleleccionado
	// y a donde se puede mover
	_moveLayer->removeChildByTag(SELECTED_PLAYER_TAG);

    for (int i = 0; i < boxTileAdded.size(); i++) {
	    _moveLayer->removeChild(boxTileAdded[i]);
    }

    boxTileAdded.clear();
}

void PlayScene::thirdPhase(CCPoint pto, CCPoint ptoConvertido)
{
	CCLOG("Entrando a: void PlayScene::thirdPhase(CCPoint pto, CCPoint ptoConvertido) \n");
	// El dado rojo fue cliqueado
	if (!LANZODADOROJO && (redDices[0]->boundingBox().containsPoint(pto) || GameState.isCurrentPlayerMachine()))
	{
		// desactiva touch
		WAIT = true;
		redDiceCallback();
		LANZODADOROJO = true;
		prevZombieLocation.x = prevZombieLocation.y = -1;
		return;
	}

	WAIT = true;
	CCPoint location, tileLocation;

	// Se verifica si el toque es para seleccionar el zombie a mover:
	if (LANZODADOROJO && prevZombieLocation.x == -1 && prevZombieLocation.y == -1 && !GameState.isCurrentPlayerMachine())
	{
		location = axisToMapCardMatrix(ptoConvertido.x,ptoConvertido.y);
		tileLocation = axisToTileMatrix(ptoConvertido.x,ptoConvertido.y);
		position p = relativeTileToAbsoluteTile(location.x, location.y, tileLocation.x,tileLocation.y);

		// si no hay zombie o si  ha sido movido antes
		if ( !GameState.isValidZombie(p) )
		{
			WAIT = false;
			return;
		}

		prevZombieLocation = tileMatrixToAxis(location.x, location.y, tileLocation.x, tileLocation.y);

		WAIT = false;

		// Muetsra en la interfaz el zombie seleccionado:
		boxTileSize = CCSizeMake(PIXELS_TILE, PIXELS_TILE);
	    boxTile = CCLayerColor::create(ccc4(185, 32, 32, 0));
	    boxTile->setPosition(ccp(prevZombieLocation.x-boxTileSize.width/2, prevZombieLocation.y-boxTileSize.height/2));
	    boxTile->setContentSize(boxTileSize);
	    boxTile->setOpacity((GLubyte)(140));
	    _moveLayer->addChild(boxTile, 1, SELECTED_ZOMBIE_TAG);

	    // Muestra en la interfaz a donde se puede mover el zombie seleccionado:
	    vector<position> posibles_moves = GameState.getPossibleZombieMoves(p);
	    CCPoint point;
	    for (int i = 0; i < posibles_moves.size(); i++) {
		    boxTile = CCLayerColor::create(ccc4(0, 119, 255, 0));

		    posibles_moves[i].t();

		    point = tileMatrixToAxis(posibles_moves[i].x/3, posibles_moves[i].y/3, posibles_moves[i].x%3, posibles_moves[i].y%3);

		    boxTile->setPosition(ccp(point.x-boxTileSize.width/2, point.y-boxTileSize.height/2));
		    boxTile->setContentSize(boxTileSize);
		    boxTile->setOpacity((GLubyte)(140));

		    boxTileAdded.push_back(boxTile);
		    _moveLayer->addChild(boxTile, 1);
	    }

	} else if (LANZODADOROJO && ((prevZombieLocation.x != -1 && prevZombieLocation.y != -1) || GameState.isCurrentPlayerMachine())) { // Va a seleccionar a donde lo quiere mover

		CCPoint nLocation, nTileLocation;
		position prev_pos, p;

		if (GameState.isCurrentPlayerMachine()) {
			decision d;
			pair<position, position> zombieMove;
			if (!d.moveZombie(GameState, zombieMove)) // No se movio zombie
			{
				WAIT = false;
				changePhase(NULL, NULL);
				return;
			}

			prev_pos = zombieMove.first;
			prev_pos.t();
			prevZombieLocation = tileMatrixToAxis(prev_pos.x / 3, prev_pos.y / 3, prev_pos.x % 3, prev_pos.y % 3);
			prev_pos.invT();

			p = zombieMove.second;
			p.t();
			nLocation.x = p.x / 3; nLocation.y = p.y / 3;
			nTileLocation.x = p.x % 3; nTileLocation.y = p.y % 3;
			ptoConvertido = tileMatrixToAxis(nLocation.x, nLocation.y, nTileLocation.x, nTileLocation.y);
			p.invT();

		} else {
			nLocation = axisToMapCardMatrix(prevZombieLocation.x,prevZombieLocation.y);
			nTileLocation = axisToTileMatrix(prevZombieLocation.x,prevZombieLocation.y);
			prev_pos = relativeTileToAbsoluteTile(nLocation.x, nLocation.y, nTileLocation.x,nTileLocation.y);

			nLocation = axisToMapCardMatrix(ptoConvertido.x,ptoConvertido.y);
			nTileLocation = axisToTileMatrix(ptoConvertido.x,ptoConvertido.y);
			p = relativeTileToAbsoluteTile(nLocation.x, nLocation.y, nTileLocation.x,nTileLocation.y);
		}

		// De-seleccion de zombie
		if ( p == prev_pos ){
			prevZombieLocation.x = -1;
			prevZombieLocation.y = -1;
			removeZombieBox();
			WAIT = false;
			return;
		}

		// Si hay zombie en el nuevo lugar o no es valido el movimiento --> no se mueve
		if ( GameState.queryZombie(p) || !canMoveZombie(prev_pos, p) )
		{
			WAIT = false;
			return;
		}

		events.clear();
		events.push_back( getEventZombieMove(prevZombieLocation, ptoConvertido) );

		GameState.moveZombieTo(prev_pos,p);

		// El movimiento es valido:

		location = axisToMapCardMatrix(prevZombieLocation.x,prevZombieLocation.y);
		tileLocation = axisToTileMatrix(prevZombieLocation.x,prevZombieLocation.y);

		// Se busca el sprite
		for (int i = 0; i < zombies.size(); i ++){
			if ( zombies[i].mapCard_i == (int) location.x     && zombies[i].mapCard_j == (int) location.y      &&
				 zombies[i].tile_i    == (int) tileLocation.x && zombies[i].tile_j    == (int) tileLocation.y)
			{
				zombieSpriteToMove = i;
				break;
			}
		}

		// Se actualiza el vector
		zombies[zombieSpriteToMove].mapCard_i = (int) nLocation.x;
		zombies[zombieSpriteToMove].mapCard_j = (int) nLocation.y;
		zombies[zombieSpriteToMove].tile_i = (int) nTileLocation.x;
		zombies[zombieSpriteToMove].tile_j = (int) nTileLocation.y;

		prevZombieLocation = zombies[zombieSpriteToMove].sprite->getPosition();

		// Se mueve el sprite. Aqui, evento != NULL
		CCFiniteTimeAction* single_action;
		CCSequence *sq;
		CCArray *actions = CCArray::createWithCapacity(7);

		NUM_OF_ZOMBIES_TO_MOVE--;

		single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::setMoveZombieSprite), (void*) &events[0]);
		actions->addObject(single_action);

		single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::animateZombieSprite), (void*) &events[0]);
		actions->addObject(single_action);

		single_action = CCDelayTime::create(abs(events[0].movement) / VELOCITY_SPRITE_MOVEMENT);
		actions->addObject(single_action);

		single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::setStaticZombieSprite));
		actions->addObject(single_action);

		if (NUM_OF_ZOMBIES_TO_MOVE == 0) {
			single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::changePhase));
			actions->addObject(single_action);
		} else if (GameState.isCurrentPlayerMachine()) {
			single_action = CCCallFuncN::create( this, callfuncN_selector(PlayScene::callThirdPhase));
			actions->addObject(single_action);
		}

		single_action = CCCallFuncND::create( this, callfuncND_selector(PlayScene::activateTouch), NULL);
		actions->addObject(single_action);

		sq = CCSequence::create(actions);

		// SIEMPRE DEBE SUCEDER que zombieSpriteToMove != null
		zombies[zombieSpriteToMove].sprite->runAction( sq );

		removeZombieBox();

	} else {
		WAIT = false;
	}
}

void PlayScene::removeZombieBox()
{
	// Se remueven los child que muestran al usuario el zombie seleleccionado
	// y a donde se puede mover
	_moveLayer->removeChildByTag(SELECTED_ZOMBIE_TAG);

    for (int i = 0; i < boxTileAdded.size(); i++) {
	    _moveLayer->removeChild(boxTileAdded[i]);
    }

    boxTileAdded.clear();
}

void PlayScene::showFirstPhase()
{
	CCLOG("Entrando a: void PlayScene::showFirstPhase()\n");
	_stayLayer->getChildByTag(MAP_CARD_DECK)->setVisible(true);
	_stayLayer->getChildByTag(MAP_CARD_REVERSE)->setVisible(false);
}

void PlayScene::hideFirstPhase()
{
	CCLOG("Entrando a: void PlayScene::hideFirstPhase()\n");
	_stayLayer->removeChildByTag(MAP_CARD_SELECTED);
	_stayLayer->getChildByTag(ROTATE_ICON)->setVisible(false);

	CCSprite * sprite = (CCSprite *) _stayLayer->getChildByTag(MAP_CARD_DECK);
	sprite->setVisible(false);
	sprite->runAction(CCFadeIn::create(1));
}

void PlayScene::showSecondPhase()
{
	CCLOG("Entrando a: void PlayScene::showSecondPhase()\n");
	redDices[lastRedDiceResult]->setVisible(true);
	blueDices[lastBlueDiceResult]->setVisible(true);
}

void PlayScene::hideSecondPhase()
{
	CCLOG("Entrando a: void PlayScene::hideSecondPhase()\n");
	blueDices[lastBlueDiceResult]->setVisible(false);
	redDices[lastRedDiceResult]->setVisible(false);

	_stayLayer->getChildByTag(CONTINUAR_LABEL_TAG)->setVisible(false);
}

void PlayScene::showThirdPhase()
{
	CCLOG("Entrando a: void PlayScene::showThirdPhase()\n");
	redDices[lastRedDiceResult]->setVisible(true);
	blueDices[lastBlueDiceResult]->setVisible(true);

	// CCMessageBox("hola!","chao");
}

void PlayScene::hideThirdPhase()
{
	CCLOG("Entrando a: void PlayScene::hideThirdPhase()\n");
	blueDices[lastBlueDiceResult]->setVisible(false);
	redDices[lastRedDiceResult]->setVisible(false);

	_stayLayer->getChildByTag(CONTINUAR_LABEL_TAG)->setVisible(false);
}


void PlayScene::setPickMapCard()
{
	CCLOG("Entrando a: void PlayScene::setPickMapCard()\n");
	// Mazo
	CCSprite* sprite;
	sprite = CCSprite::create("mazo.png");
	sprite->setPosition(ccp(VisibleRect::leftBottom().x + sprite->getContentSize().width/2,
							  VisibleRect::leftBottom().y + sprite->getContentSize().height/2 ));
	sprite->setTag(MAP_CARD_DECK);
	sprite->setVisible(false);
	_stayLayer->addChild(sprite);

	// reversed mapCard
	sprite = CCSprite::create("reverse_map_card.png");
	sprite->setPosition(ccp(VisibleRect::leftBottom().x + sprite->getContentSize().width/2 + 10,
							VisibleRect::leftBottom().y + sprite->getContentSize().height/2 + 10));
	sprite->setTag(MAP_CARD_REVERSE);
	sprite->setVisible(false);
	_stayLayer->addChild(sprite);

	// Rotate
	int offset_x = sprite->getContentSize().width / 2 + 8;
	int offset_y = sprite->getPositionY();

	sprite = CCSprite::create("rotate.png");
	sprite->setPosition(ccp(VisibleRect::bottom().x + sprite->getContentSize().width/2 + offset_x,
							offset_y));
	sprite->setTag(ROTATE_ICON);
	sprite->setVisible(false);
	_stayLayer->addChild(sprite);

}

void PlayScene::initFlipXCallback(CCObject* target)
{
	CCLOG("Entrando a: void PlayScene::initFlipXCallback(CCObject* target)\n");
	CCSprite * reversed_sprite = (CCSprite *) _stayLayer->getChildByTag(MAP_CARD_REVERSE);

	mapCard card = GameState.pickMapCard();

	// Obtener posibles posiciones
	allowedPositions.clear();
	allowedPositions = GameState.getAllPosibleMapCard(GameState.getLastMapCard());

	reversed_sprite->setVisible(true);
	CCSprite * sprite = CCSprite::create(card.getPath().c_str());

    // 110 y 300
	sprite->setScale(reversed_sprite->getContentSize().width/PIXELS_MAP_CARD);

	sprite->setPosition(ccp(VisibleRect::bottom().x,
			                VisibleRect::bottom().y + reversed_sprite->getContentSize().height/2));
	sprite->setTag(MAP_CARD_SELECTED);
	sprite->setVisible(false);
	_stayLayer->addChild(sprite);

	CCActionInterval*  moveToBottom = CCMoveTo::create(1, sprite->getPosition());
    CCOrbitCamera* camera = CCOrbitCamera::create(1, 1, 0, 0, 90, 0, 0);
    CCHide* hide = CCHide::create();
    CCCallFunc* func = CCCallFunc::create(this, callfunc_selector(PlayScene::show_mapCard_selected));
    CCActionInterval* action = (CCActionInterval*) CCSequence::create(moveToBottom, camera, hide, func, NULL);
    reversed_sprite->runAction(action);
}

void PlayScene::show_mapCard_selected(CCNode* node)
{
	CCLOG("Entrando a: void PlayScene::show_mapCard_selected(CCNode* node)\n");
	CCSprite * icon = (CCSprite *) _stayLayer->getChildByTag(ROTATE_ICON);
	icon->setVisible(true);

	CCSprite * sprite = (CCSprite *) _stayLayer->getChildByTag(MAP_CARD_SELECTED);
    CCShow* show = CCShow::create();
    CCOrbitCamera* camera = CCOrbitCamera::create(1, 1, 0, 270, 90, 0, 0);
    CCActionInterval* action = (CCActionInterval*) CCSequence::create(show, camera, NULL);
    sprite->runAction(action);

    camera = CCOrbitCamera::create(1, 1, 0, 0, 0, 0, 0);
    sprite = (CCSprite * ) _stayLayer->getChildByTag(MAP_CARD_REVERSE);
	sprite->setPosition(ccp(VisibleRect::leftBottom().x + sprite->getContentSize().width/2 + 10,
							VisibleRect::leftBottom().y + sprite->getContentSize().height/2 + 10));

	// Si la carta no puede ser colocada
	bool cantBePlaced = allowedPositions[0].size() == 0 &&
						allowedPositions[1].size() == 0 &&
						allowedPositions[2].size() == 0 &&
						allowedPositions[3].size() == 0;

	sprite->runAction(CCSequence::create(camera,
			CCCallFuncND::create( this, callfuncND_selector(PlayScene::activateTouch), NULL),
			CCCallFuncND::create( this, callfuncND_selector(PlayScene::changeSubPhase), NULL),
			(cantBePlaced ?
					(GameState.isCurrentPlayerMachine() ? NULL :
														  CCCallFuncN::create( this, callfuncN_selector(PlayScene::popupLayer))) :
														  NULL
			),
			NULL));

	sprite = (CCSprite *) _stayLayer->getChildByTag(MAP_CARD_DECK);
	sprite->runAction(CCFadeOut::create(1));

    PUTMAPCARD = true;

    if (!GameState.isCurrentPlayerMachine())
    	addMapCardBox();

}

void PlayScene::addMapCardBox()
{
	// Muestra en la interfaz las posibles posiciones del mapCard
	CCPoint point;

	for (int i = 0; i < allowedPositions[currCardRotation].size(); i++) {
		boxTile = CCLayerColor::create(ccc4(0, 119, 255, 0));

		allowedPositions[currCardRotation][i].t();

		point = tileMatrixToAxis(allowedPositions[currCardRotation][i].x/3, allowedPositions[currCardRotation][i].y/3, allowedPositions[currCardRotation][i].x%3, allowedPositions[currCardRotation][i].y%3);

		allowedPositions[currCardRotation][i].invT();

		boxTile->setPosition(ccp(point.x-boxMapCardSize.width/2, point.y-boxMapCardSize.height/2));
		boxTile->setContentSize(boxMapCardSize);
		boxTile->setOpacity((GLubyte)(140));

		boxTileAdded.push_back(boxTile);
		_moveLayer->addChild(boxTile, 1);
	}
	// FIN: Muestra en la interfaz las posibles posiciones del mapCard
}

void PlayScene::removeMapCardBox()
{
	// Se remueven los child que muestran posibles posiciones del mapcard
    for (int i = 0; i < boxTileAdded.size(); i++) {
	    _moveLayer->removeChild(boxTileAdded[i]);
    }

    boxTileAdded.clear();
}

void PlayScene::rotate_mapCard()
{
	CCLOG("Entrando a: void PlayScene::rotate_mapCard()\n");
	currCardRotation = (currCardRotation + 1) % 4;

	CCSprite * sprite = (CCSprite *) _stayLayer->getChildByTag(MAP_CARD_SELECTED);
	CCRotateTo* rotate_action = CCRotateTo::create(1, 90 * currCardRotation);
	sprite->runAction(CCSequence::create(rotate_action,
			CCCallFuncND::create( this, callfuncND_selector(PlayScene::activateTouch), NULL),
			NULL));

	removeMapCardBox();
	addMapCardBox();
}

void PlayScene::setBackgrounds()
{
	CCLOG("Entrando a: void PlayScene::setBackgrounds()\n");
	float h;
	CCLabelTTF * label = (CCLabelTTF *) _stayLayer->getChildByTag(TITLE_LABEL_TAG);
	h = label->getContentSize().height;

	label = (CCLabelTTF *) _stayLayer->getChildByTag(SUBTITLE_LABEL_TAG);
	h += label->getContentSize().height + 3;

    CCSize boxSize = CCSizeMake(VisibleRect::getVisibleRect().size.width, h);

    CCLayerColor *box = CCLayerColor::create(ccc4(0, 0, 0, 0));
    box->setPosition(ccp(VisibleRect::top().x-boxSize.width/2, VisibleRect::top().y-h));
    box->setContentSize(boxSize);
    box->setOpacity((GLubyte)(140));
    _stayLayer->addChild(box,1);
}

void PlayScene::checkBattle(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::checkBattle(CCNode* sender, void* data)\n");
	if (gameIsOver) return;

	if ( GameState.queryZombie() )
	{
		HAY_BATALLA = true;
		msg = "A batallar ! Lanza el dado rojo";
		putSubtitleInformation(NULL, NULL);

	} else if ( LANZODADOAZUL ) {
		checkLifeAndBullet(NULL, NULL);
		checkLeftMoves(NULL, NULL);
		return;
	}

	if (GameState.isCurrentPlayerMachine() && currFase == 1){
		secondPhase(ccp(-1,-1), ccp(-1,-1));
	}

}

void PlayScene::checkLeftMoves(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::checkLeftMoves()\n");
	if (possibleMoves.size() == 0) {
		possibleMoves = GameState.getPossibleMoves().first;

		// No hay movimientos restantes
		if (possibleMoves.size() == 0) {
			changePhase(NULL, NULL);
			return;
		}

		// Se coloca la interfaz del usuario
		showCurrPlayerBox();
		addPlayerBox();

		if (GameState.isCurrentPlayerMachine()){
			secondPhase(ccp(-1, -1), ccp(-1, -1));
		} else
			showContinueButton(NULL, NULL);
	}
}

void PlayScene::checkLifeAndBullet(CCNode* sender, void* data)
{
	CCLOG("Entrando a: void PlayScene::checkLifeAndBullet(CCNode* sender, void* data)\n");
	CCPoint point;
	CCPoint ij_cardMap = axisToMapCardMatrix(mSprite[GameState.getCurrentPlayer()]->getPositionX(), mSprite[GameState.getCurrentPlayer()]->getPositionY());
	CCPoint ij_tile = axisToTileMatrix(mSprite[GameState.getCurrentPlayer()]->getPositionX(), mSprite[GameState.getCurrentPlayer()]->getPositionY());

	if ( GameState.queryLife() )
	{
		for (int i = 0; i < lifes.size(); i++) {
			if ( lifes[i].mapCard_i == (int)ij_cardMap.x &&  lifes[i].mapCard_j == (int)ij_cardMap.y &&
					lifes[i].tile_i == (int)ij_tile.x &&  lifes[i].tile_j == (int)ij_tile.y) {

				point = _moveLayer->convertToNodeSpace( _stayLayer->getChildByTag(LIFE_ICON_TAG)->getPosition() );

				lifes[i].sprite->runAction(CCSequence::create(CCMoveTo::create(1, point),
	                                          CCCallFuncN::create( this, callfuncN_selector(PlayScene::removeSprite)),
						                      NULL));

				lifes[i] = lifes[lifes.size()-1];
				break;
			}
		}

		modifyPlayerLifes(1);
		lifes.pop_back();

	} else if ( GameState.queryBullet() ) {

		for (int i = 0; i < bullets.size(); i++) {
			if ( bullets[i].mapCard_i == (int)ij_cardMap.x &&  bullets[i].mapCard_j == (int)ij_cardMap.y &&
					bullets[i].tile_i == (int)ij_tile.x &&  bullets[i].tile_j == (int)ij_tile.y) {
				point = _moveLayer->convertToNodeSpace( _stayLayer->getChildByTag(BULLET_ICON_TAG)->getPosition() );

				bullets[i].sprite->runAction(CCSequence::create(CCMoveTo::create(1, point),
	                                            CCCallFuncN::create( this, callfuncN_selector(PlayScene::removeSprite)),
						                        NULL));

				bullets[i] = bullets[bullets.size()-1];
				break;
			}
		}

		modifyPlayerBullets(1);
		bullets.pop_back();

	}
}

void PlayScene::winBattle(CCNode* sender, void * data)
{
	CCLOG("Entrando a: void PlayScene::winBattle(CCNode* sender, void * data)\n");

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/zombie_die.wav");

	msg = "Has ganado...";
	putSubtitleInformation(NULL, NULL);

	HAY_BATALLA = false;

	CCPoint ij_cardMap = axisToMapCardMatrix(mSprite[GameState.getCurrentPlayer()]->getPositionX(), mSprite[GameState.getCurrentPlayer()]->getPositionY());
	CCPoint ij_tile = axisToTileMatrix(mSprite[GameState.getCurrentPlayer()]->getPositionX(), mSprite[GameState.getCurrentPlayer()]->getPositionY());
	CCPoint point;
	CCArray * actions = CCArray::createWithCapacity(8);
	CCSprite * sprite;

	for (int i = 0; i < zombies.size(); i++) {
		if ( zombies[i].mapCard_i == (int)ij_cardMap.x &&  zombies[i].mapCard_j == (int)ij_cardMap.y &&
		     zombies[i].tile_i == (int)ij_tile.x &&  zombies[i].tile_j == (int)ij_tile.y) {

			sprite = zombies[i].sprite;
			zombies[i] = zombies[zombies.size()-1];
			break;
		}
	}

	checkLifeAndBullet(NULL, NULL);

	point = _moveLayer->convertToNodeSpace( _stayLayer->getChildByTag(ZOMBIE_ICON_TAG)->getPosition() );
	actions->addObject(CCMoveTo::create(1, point));

	if (GameState.getCurrentPlayerZombie() + 1 == ZOMBIES_TO_WIN || GameState.currentPlayerOverHeliport()) {
		actions->addObject(CCCallFunc::create( this, callfunc_selector(PlayScene::gameOver)));
	} else {
		if ( LANZODADOAZUL ) {
			actions->addObject(CCCallFuncN::create( this, callfuncN_selector(PlayScene::checkLeftMoves)));
		} else {
			msg = name[1][0];
			actions->addObject(CCCallFuncND::create( this, callfuncND_selector(PlayScene::putSubtitleInformation), NULL));
			if (GameState.isCurrentPlayerMachine())
				actions->addObject(CCCallFuncN::create( this, callfuncN_selector(PlayScene::callSecondPhase)));
		}

		zombies.pop_back();
		modifyPlayerZombies();

		actions->addObject(CCCallFuncN::create( this, callfuncN_selector(PlayScene::removeSprite)));
	}

	sprite->runAction(CCSequence::create(actions));
}

void PlayScene::removeSprite(CCNode* sender, void * data)
{
	CCLOG("Entrando a: void PlayScene::removeSprite(CCNode* sender, void * data)\n");
	_moveLayer->removeChild(sender);
}

void PlayScene::loseBattleAndDie(CCNode* sender)
{
	CCLOG("Entrando a: void PlayScene::loseBattleAndDie(CCNode* sender)\n");
	msg = "Has perdido ...";
	putSubtitleInformation(NULL, NULL);
	HAY_BATALLA = false;

	CCPoint point = tileMatrixToAxis(MAX_MAP_DIM/2,MAX_MAP_DIM/2,1,1);

	point.y += (mSprite[0]->getContentSize().height / 2) + 5;
	if (GameState.getCurrentPlayer() == 0)
		point.x += (mSprite[GameState.getCurrentPlayer()]->getContentSize().width / 2) + 7;
	else
		point.x -= (mSprite[GameState.getCurrentPlayer()]->getContentSize().width / 2) + 7;

	float distance = mSprite[GameState.getCurrentPlayer()]->getPosition().getDistance(point);
	mSprite[GameState.getCurrentPlayer()]->runAction(CCSequence::create(CCMoveTo::create(distance/VELOCITY_SPRITE_MOVEMENT, point),
			CCCallFuncN::create( this, callfuncN_selector(PlayScene::changePhase)),
			NULL));

//	_moveLayer->runAction(CCFollow::create(mSprite[GameState.getCurrentPlayer()],
//			  CCRectMake(0,0,_moveLayer->getContentSize().width, _moveLayer->getContentSize().height)));

	GameState.killcurrentPlayer();

	modifyScreenLifes();
	modifyScreenBullets();
	modifyScreenZombies();
}

void PlayScene::continueBattle(CCNode* sender)
{
	CCLOG("Entrando a: void PlayScene::continueBattle(CCNode* sender)\n");
	msg = "Toma una decisión ...";
	putSubtitleInformation(NULL, NULL);

	_stayLayer->getChildByTag(QUESTION_LABEL_TAG)->setVisible(true);

	if (GameState.getCurrentPlayerLife() > 0)
		_stayLayer->getChildByTag(QUESTION_LIFE_ICON_TAG)->setVisible(true);

	if (GameState.getCurrentPlayerBullet() + lastRedDiceResult >= 3)
		_stayLayer->getChildByTag(QUESTION_BULLET_ICON_TAG)->setVisible(true);

	HAY_PREGUNTA = true;

	if (GameState.isCurrentPlayerMachine()){
		secondPhase(ccp(-1,-1), ccp(-1,-1));
	}
}

void PlayScene::gameOver()
{
	CCLOG("Entrando a: void PlayScene::gameOver() \n");

	gameIsOver = true;

	// Limpiamos las interfaces
	_moveLayer->removeAllChildren();
	_stayLayer->removeAllChildren();

	this->removeChild(_moveLayer);
	this->removeChild(_stayLayer);

	CCLayer * _gameOverLayer = CCLayer::create();
	char endMsg[30];
	sprintf(endMsg, "Ganó el jugador %d", GameState.getCurrentPlayer() + 1);
	CCLabelTTF * label = CCLabelTTF::create("GAME OVER", "Arial", FONT_SIZE + 20);
	label->setPosition(ccp(VisibleRect::center().x, VisibleRect::center().y + 20));

	int off_y = label->getContentSize().height/2 + 10;
	_gameOverLayer->addChild(label);

	label = CCLabelTTF::create(endMsg, "Arial", FONT_SIZE + 10);
	label->setPosition(ccp(VisibleRect::center().x, VisibleRect::center().y - off_y + 20));
	_gameOverLayer->addChild(label);

	setMenuBackMenu(_gameOverLayer);

	this->addChild(_gameOverLayer);

	setTouchEnabled(false);
}


void PlayScene::popupLayer(CCNode* sender, void * data)
{
    PopupLayer* pl = PopupLayer::create("map/popupBackground.png");

    pl->setContentSize(CCSizeMake(300, 250));
    pl->setTitle("Carta sin lugar");
    pl->setContentText("La carta de mapa no puede ser colocada en el mapa.", 20, 60, 250);

    pl->setCallbackFunc(this, callfuncN_selector(PlayScene::popUpbuttonCallback));

    pl->addButton("map/buttonBackground.png", "map/buttonBackground.png", "Continuar", 1);

    this->addChild(pl);
}

void PlayScene::popUpbuttonCallback(CCNode *pNode)
{
    changePhase(NULL,NULL);
}

void PlayScene::showContinueButton(CCNode* sender, void * data)
{
	if (!GameState.isCurrentPlayerMachine())
		_stayLayer->getChildByTag(CONTINUAR_LABEL_TAG)->setVisible(true);
}

// LLamadas para la maquina
void PlayScene::callSecondPhase(CCNode* sender, void * data)
{
	secondPhase(ccp(-1, -1), ccp(-1, -1));
}

void PlayScene::callThirdPhase(CCNode* sender, void * data)
{
	thirdPhase(ccp(-1, -1), ccp(-1, -1));
}
