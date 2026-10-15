#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "R387Popup.hpp"

using namespace geode::prelude;

class $modify(R387EditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        this->addChild(menu, 999);

        auto sprite = ButtonSprite::create("R387", 48, 0, 1.f, true, "goldFont.fnt", "GJ_button_02.png", 0.f);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(R387EditorUI::onOpenR387));
        auto size = CCDirector::sharedDirector()->getWinSize();
        button->setPosition({size.width - 43.f, 82.f});
        button->setID("r387-agent-button"_spr);
        menu->addChild(button);

        log::info("R387 Agent editor button initialized.");
        return true;
    }

    void onOpenR387(CCObject*) {
        auto popup = R387Popup::create(this);
        if (popup) popup->show();
    }
};
