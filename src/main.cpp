#include <Geode/Geode.hpp>
#include <csv2/writer.hpp>
#include <vector>
#include <string>

using namespace csv2;
using namespace geode::prelude;

bool hackFix = false;

#include <Geode/modify/LevelListLayer.hpp>
class $modify(CSVListLayer, LevelListLayer) {
	bool init(GJLevelList* p0) {
		if (!LevelListLayer::init(p0)) return false;

		auto btn = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png"), this, menu_selector(CSVListLayer::downloadList)
		);

		static_cast<CCMenu*>(getChildByID("right-side-menu"))->addChild(btn);
		static_cast<CCMenu*>(getChildByID("right-side-menu"))->updateLayout();

		return true;
	}

	void downloadList(cocos2d::CCObject*) {
		std::ofstream stream(fmt::format("{}.csv", (Mod::get()->getConfigDir() / std::string(m_levelList->m_listName)).string()));
		Writer<delimiter<','>> writer(stream);

		std::vector<std::vector<std::string>> rows = 
        {
            {"Name", "Creator", "ID", "Rating", "Song ID"}
        };

		for (int id : m_levelList->m_levels) {
			CCObject* levelObj = m_levelList->m_levelsDict->objectForKey(std::to_string(id));
			GJGameLevel* level = static_cast<GJGameLevel*>(levelObj);

			rows.push_back({std::string(level->m_levelName), std::string(level->m_creatorName), std::to_string(level->m_levelID), std::to_string(level->m_stars), std::to_string(level->m_songID) });
		}

		writer.write_rows(rows);
		stream.close();
	}
};

#include <Geode/modify/LevelListCell.hpp>
class $modify(AddButtonLLC, LevelListCell) {
	void loadFromList(GJLevelList* list) { // 15 and -20 y vals
		LevelListCell::loadFromList(list);

		if (m_addingLevel) {
			CCMenu* mainMenu = static_cast<CCMenu*>(m_mainLayer->getChildByID("main-menu"));
			mainMenu->getChildByID("view-button")->setVisible(false);

			ButtonSprite* bottomSpr = ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_02.png", 0.5f);
			bottomSpr->setScale(0.75f);
			CCMenuItemSpriteExtra* addBottom = CCMenuItemSpriteExtra::create(bottomSpr, this, menu_selector(AddButtonLLC::onAddToBottom));
			addBottom->setPositionY(-20.f);
			mainMenu->addChild(addBottom);

			ButtonSprite* topSpr = ButtonSprite::create("Add", "bigFont.fnt", "GJ_button_02.png", 0.5f);
			topSpr->setScale(0.75f);
			CCMenuItemSpriteExtra* addTop = CCMenuItemSpriteExtra::create(topSpr, this, menu_selector(AddButtonLLC::onAddToTop));
			addTop->setPositionY(15.f);
			mainMenu->addChild(addTop);
		}
	}

	void onAddToBottom(CCObject* target) {
		LevelListCell::onClick(target);
	}

	void onAddToTop(CCObject* target) {
		// TODO: move the last level to the beginning instead of doing this node IDs bullshit
		LevelInfoLayer* lil = static_cast<LevelInfoLayer*>(CCDirector::get()->getRunningScene()->getChildByID("LevelInfoLayer"));
		LevelListCell::onClick(target);
		m_levelList->reorderLevel(lil->m_level->m_levelID, 0);
	}
};

#include <Geode/modify/LevelListLayer.hpp>
class $modify(ScrollLLL, LevelListLayer) {
	bool init(GJLevelList* list) {
		if (!LevelListLayer::init(list)) return false;

		auto winSize = CCDirector::get()->getWinSize();

		CCSprite* upSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
		CCMenuItemSpriteExtra* scrollUp = CCMenuItemSpriteExtra::create(upSpr, this, menu_selector(ScrollLLL::onScrollUp));
		upSpr->setScale(0.75f);
		upSpr->setRotation(90.f);
		scrollUp->setPosition(ccp((winSize.width / 2.f) + 220.f, (winSize.height / 2.f) + 20.f));

		CCSprite* downSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
		CCMenuItemSpriteExtra* scrollDown = CCMenuItemSpriteExtra::create(downSpr, this, menu_selector(ScrollLLL::onScrollDown));
		downSpr->setScale(0.75f);
		downSpr->setRotation(-90.f);
		scrollDown->setPosition(ccp((winSize.width / 2.f) + 220.f, (winSize.height / 2.f) - 20.f));

		CCMenu* menu = CCMenu::create();
		menu->setPosition(ccp(0,0));
		menu->addChild(scrollUp);
		menu->addChild(scrollDown);

		addChild(menu);
		return true;
	}

	void onScrollUp(CCObject*) {
		TableView* table = this->getChildByID("GJListLayer")->getChildByID("list-view")->getChildByType<TableView>(0);
		CCContentLayer* content = table->getChildByType<CCContentLayer>(0);
		CCPoint to = ccp(content->getPositionX(), -content->getContentHeight() + table->getContentHeight() - (table->getChildrenCount() / 2.f));
		if (!Mod::get()->getSettingValue<bool>("instant-scroll")) {
			CCEaseExponentialInOut* acc = CCEaseExponentialInOut::create(CCMoveTo::create(0.2f, to));
			content->runAction(acc);
		} else {
			content->setPosition(to);
		}
	}

	void onScrollDown(CCObject*) {
		CCContentLayer* content = this->getChildByID("GJListLayer")->getChildByID("list-view")->getChildByType<TableView>(0)->getChildByType<CCContentLayer>(0);
		if (!Mod::get()->getSettingValue<bool>("instant-scroll")) {
			CCEaseExponentialInOut* acc = CCEaseExponentialInOut::create(CCMoveTo::create(0.2f, ccp(content->getPositionX(), 2.f)));
			content->runAction(acc);	
		} else {
			content->setPosition(ccp(content->getPositionX(), 2.f));
		}
	}
};
/*
#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(LevelInfoLayer) {
	bool cellPerformedAction(TableViewCell* p0, int p1, CellAction p2, cocos2d::CCNode* p3) {
		if (CCDirector::get()->getRunningScene()->getChildByID("LevelListLayer") && p1 == 0x1e && p2 == (CellAction)1) {
			static_cast<LevelListCell*>(p0)->m_levelList->addLevelToList(m_level);
			return true;
		}
		return LevelInfoLayer::cellPerformedAction(p0, p1, p2, p3);
	}
};

#include <Geode/modify/LevelCell.hpp>
class $modify(AddToListLLC, LevelCell) {
	void loadFromLevel(GJGameLevel* lvl) {
		LevelCell::loadFromLevel(lvl);

		if (m_compactView) {
			CCMenuItemSpriteExtra* addTo = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_listAddBtn_001.png"), LevelInfoLayer::create(lvl, false), menu_selector(LevelInfoLayer::onAddToList));
			addTo->setPosition(ccp(m_mainLayer->getChildByID("main-menu")->getChildByID("delete-button")->getPositionX() - 40.f, m_mainLayer->getChildByID("main-menu")->getChildByID("delete-button")->getPositionY()));
			addTo->setID("add-to-button"_spr);
			addTo->setVisible(false);
			m_mainLayer->getChildByID("main-menu")->addChild(addTo);
		}
	}

	void onAddToList(CCObject* target) {
		char[offsetof(LevelBrowserLayer, m_list)] a;
		a = 0;
		GJSearchObject* search = GJSearchObject::create((SearchType)0x66);
		search->m_searchMode = 0x1;
		LevelBrowserLayer* lbl = LevelBrowserLayer::create(search);
		LevelInfoLayer* lil = LevelInfoLayer::create(reinterpret_cast<GJGameLevel*>(target), false);
		lbl->m_delegate = lil;
		lbl->show();

	}
};

#include <Geode/modify/LevelListLayer.hpp>
class $modify(AddToListLLL, LevelListLayer) {
	void onToggleEditMode(CCObject* target) {
		LevelListLayer::onToggleEditMode(target);
		CCArrayExt<LevelListCell*> cells = getChildByID("GJListLayer")->getChildByID("list-view")->getChildByType<TableView>(0)->getChildByType<CCContentLayer>(0)->getChildren();
		for (LevelListCell* cell : cells) {
			cell->m_mainLayer->getChildByID("main-menu")->getChildByID("add-to-button"_spr)->setVisible(m_editMode == 2);
		}
	}
};

*/