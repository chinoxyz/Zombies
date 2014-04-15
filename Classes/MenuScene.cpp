/*
 * MenuScene.cpp
 *
 *  Created on: 02/04/2014
 *      Author: hector
 */

#include "MenuScene.h"
#include "CreditsScene.h"
#include "PlayScene.h"
#include "Resources.h"
#include "VisibleRect.h"
#include "support/CCPointExtension.h"

USING_NS_CC;

static const int LINE_SPACE = 40;
static CCPoint s_tCurPos = CCPointZero;

CCScene* MenuScene::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();

    // 'layer' is an autorelease object
    MenuScene *layer = MenuScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

bool MenuScene::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }

	menuOptions = CCMenu::create();

	for ( int i = 0;  i < MENU_OPTIONS_COUNT; ++ i) {
		CCLabelTTF* label = CCLabelTTF::create(MENU_OPTIONS[i].c_str(), "Arial", FONT_SIZE);
		CCMenuItemLabel* itemLabel = CCMenuItemLabel::create(label,this,menu_selector(MenuScene::menuCallBack));
		menuOptions->addChild(itemLabel,i);
		itemLabel->setPosition( ccp( VisibleRect::center().x , VisibleRect::center().y + ((MENU_OPTIONS_COUNT/2 + MENU_OPTIONS_COUNT%2 - (i+1)) * LINE_SPACE) ) );
	}

	menuOptions->setContentSize(CCSizeMake(VisibleRect::getVisibleRect().size.width, (MENU_OPTIONS_COUNT + 1) * (LINE_SPACE)));
	menuOptions->setPosition(s_tCurPos);
	addChild(menuOptions);
	setTouchEnabled(true);

	return true;
}

void MenuScene::menuCallBack(CCObject * pSender)
{
	// get the userdata, it's the index of the menu item clicked
	CCMenuItem* pMenuItem = (CCMenuItem *)(pSender);
	int nIdx = pMenuItem->getZOrder();

	// create the test scene and run it
	switch (nIdx)
	{
	case PLAY:
		play();
		break;
	case CREDITS:
		credits();
		break;
	case EXIT:
		close();
		break;
	default:
		break;
	}
}

void MenuScene::close()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	CCMessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");

	#else
		CCDirector::sharedDirector()->end();
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		exit(0);
	#endif
#endif
}

void MenuScene::credits()
{
	CCScene * creditsScene = CreditsScene::scene();

	if (creditsScene)
	{
		CCDirector::sharedDirector()->replaceScene(creditsScene);
	}
}

void MenuScene::play()
{
	CCScene * playScene = PlayScene::scene();

	if (playScene)
	{
		CCDirector::sharedDirector()->replaceScene(playScene);
	}
}