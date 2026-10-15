#include "R387Popup.hpp"
#include "WindowsImageLoader.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <set>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
constexpr float kPopupW = 492.f;
constexpr float kPopupH = 326.f;
constexpr float kPreviewW = 224.f;
constexpr float kPreviewH = 146.f;
constexpr float kTileSize = 30.f;

std::string basenameForLabel(const std::filesystem::path& path) {
    if (path.empty()) return "No image selected";
    auto name = path.filename().string();
    if (name.size() > 29) name = "..." + name.substr(name.size() - 26);
    return name;
}

std::vector<GameObject*> currentSelection(EditorUI* ui) {
    std::vector<GameObject*> result;
    if (!ui) return result;
    std::set<int> uniqueIDs;
    if (ui->m_selectedObject) {
        result.push_back(ui->m_selectedObject);
        uniqueIDs.insert(ui->m_selectedObject->m_uniqueID);
    }
    if (ui->m_selectedObjects) {
        for (auto object : geode::cocos::CCArrayExt<GameObject, false>(ui->m_selectedObjects)) {
            if (object && uniqueIDs.insert(object->m_uniqueID).second) result.push_back(object);
        }
    }
    return result;
}

CCMenuItemSpriteExtra* makeButton(const char* text, int width, CCObject* target, SEL_MenuHandler callback) {
    auto sprite = ButtonSprite::create(text, width, 0, 1.f, true, "goldFont.fnt", "GJ_button_01.png", 0.f);
    return CCMenuItemSpriteExtra::create(sprite, target, callback);
}

void addLabel(CCNode* parent, const char* text, CCPoint pos, float scale = 0.45f, ccColor3B color = {235, 240, 255}) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(scale);
    label->setColor(color);
    label->setPosition(pos);
    parent->addChild(label);
}
}

R387Popup* R387Popup::create(EditorUI* editorUI) {
    auto popup = new R387Popup();
    if (popup->init(editorUI)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool R387Popup::init(EditorUI* editorUI) {
    if (!Popup::init(kPopupW, kPopupH)) return false;
    m_editorUI = editorUI;
    m_settings.objectLimit = 750;
    this->setTitle("R387 Agent");

    // Two-column, dark editor: selection/upload controls at left; conversion preview at right.
    auto leftPanel = CCLayerColor::create({14, 20, 34, 245}, 218.f, 222.f);
    leftPanel->setPosition({18.f, 48.f});
    m_mainLayer->addChild(leftPanel, -1);

    auto rightPanel = CCLayerColor::create({14, 20, 34, 245}, 238.f, 222.f);
    rightPanel->setPosition({242.f, 48.f});
    m_mainLayer->addChild(rightPanel, -1);

    addLabel(m_mainLayer, "IMAGE TO EDITABLE OBJECTS", {245.f, 263.f}, 0.45f, {83, 214, 255});
    addLabel(m_mainLayer, "SELECTION", {127.f, 245.f}, 0.42f, {136, 153, 185});
    addLabel(m_mainLayer, "IMAGE", {127.f, 179.f}, 0.42f, {136, 153, 185});
    addLabel(m_mainLayer, "OBJECT BUDGET", {127.f, 118.f}, 0.42f, {136, 153, 185});
    addLabel(m_mainLayer, "LIVE PREVIEW", {361.f, 245.f}, 0.42f, {136, 153, 185});

    m_selectionLabel = CCLabelBMFont::create("0 selected", "bigFont.fnt");
    m_selectionLabel->setScale(0.48f);
    m_selectionLabel->setColor({245, 247, 255});
    m_selectionLabel->setPosition({127.f, 225.f});
    m_mainLayer->addChild(m_selectionLabel);

    m_imageLabel = CCLabelBMFont::create("No image selected", "bigFont.fnt");
    m_imageLabel->setScale(0.38f);
    m_imageLabel->setColor({198, 208, 230});
    m_imageLabel->setPosition({127.f, 157.f});
    m_mainLayer->addChild(m_imageLabel);

    m_limitLabel = CCLabelBMFont::create("Limit: 750", "bigFont.fnt");
    m_limitLabel->setScale(0.48f);
    m_limitLabel->setColor({255, 255, 255});
    m_limitLabel->setPosition({127.f, 98.f});
    m_mainLayer->addChild(m_limitLabel);

    m_statusLabel = CCLabelBMFont::create("Ready · local processing", "bigFont.fnt");
    m_statusLabel->setScale(0.36f);
    m_statusLabel->setColor({105, 221, 173});
    m_statusLabel->setPosition({245.f, 36.f});
    m_mainLayer->addChild(m_statusLabel);

    auto previewBG = CCLayerColor::create({5, 9, 18, 255}, kPreviewW, kPreviewH);
    previewBG->setPosition({249.f, 91.f});
    m_mainLayer->addChild(previewBG);
    m_previewGrid = CCNode::create();
    m_previewGrid->setContentSize({kPreviewW, kPreviewH});
    m_previewGrid->setPosition({249.f, 91.f});
    m_mainLayer->addChild(m_previewGrid, 2);

    m_statsLabel = CCLabelBMFont::create("Choose an image to preview", "bigFont.fnt");
    m_statsLabel->setScale(0.35f);
    m_statsLabel->setColor({177, 190, 215});
    m_statsLabel->setPosition({361.f, 77.f});
    m_mainLayer->addChild(m_statsLabel);

    auto menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});
    m_mainLayer->addChild(menu, 5);

    auto choose = makeButton("CHOOSE IMAGE", 112, this, menu_selector(R387Popup::onChooseImage));
    choose->setPosition({127.f, 197.f});
    menu->addChild(choose);

    auto down = makeButton("-", 34, this, menu_selector(R387Popup::onLimitDown));
    down->setPosition({69.f, 68.f});
    menu->addChild(down);
    auto up = makeButton("+", 34, this, menu_selector(R387Popup::onLimitUp));
    up->setPosition({185.f, 68.f});
    menu->addChild(up);
    auto unlimited = makeButton("UNLIMITED", 100, this, menu_selector(R387Popup::onUnlimited));
    unlimited->setPosition({127.f, 40.f});
    menu->addChild(unlimited);

    auto generate = makeButton("GENERATE DECORATION", 168, this, menu_selector(R387Popup::onGenerate));
    generate->setPosition({361.f, 40.f});
    menu->addChild(generate);

    refreshLabels();
    refreshPreview();
    return true;
}

void R387Popup::showError(const std::string& title, const std::string& detail) {
    FLAlertLayer::create(title.c_str(), detail.c_str(), "OK")->show();
}

void R387Popup::onChooseImage(CCObject*) {
    const auto path = r387::chooseImageFile();
    if (path.empty()) return; // Cancel is not an error.
    auto loaded = r387::loadImageWindows(path);
    if (!loaded) {
        showError("Could not load image", loaded.error);
        return;
    }
    m_image = std::move(loaded.image);
    m_imagePath = path;
    m_statusLabel->setString("Image loaded · processing locally");
    refreshLabels();
    refreshPreview();
}

void R387Popup::onLimitDown(CCObject*) {
    if (m_settings.unlimited) m_settings.unlimited = false;
    const std::size_t limits[] = {1, 25, 50, 100, 250, 500, 750, 1000, 1500, 2000, 2500};
    for (std::size_t i = 0; i < std::size(limits); ++i) {
        if (limits[i] >= m_settings.objectLimit) {
            m_settings.objectLimit = i == 0 ? limits[0] : limits[i - 1];
            break;
        }
    }
    refreshLabels();
    refreshPreview();
}

void R387Popup::onLimitUp(CCObject*) {
    if (m_settings.unlimited) return;
    const std::size_t limits[] = {1, 25, 50, 100, 250, 500, 750, 1000, 1500, 2000, 2500};
    for (std::size_t i = 0; i < std::size(limits); ++i) {
        if (limits[i] > m_settings.objectLimit) {
            m_settings.objectLimit = limits[i];
            break;
        }
    }
    refreshLabels();
    refreshPreview();
}

void R387Popup::onUnlimited(CCObject*) {
    m_settings.unlimited = !m_settings.unlimited;
    refreshLabels();
    refreshPreview();
}

void R387Popup::refreshLabels() {
    const auto selection = currentSelection(m_editorUI);
    m_selectionLabel->setString(fmt::format("{} object{} selected", selection.size(), selection.size() == 1 ? "" : "s").c_str());
    m_imageLabel->setString(basenameForLabel(m_imagePath).c_str());
    if (m_settings.unlimited) {
        m_limitLabel->setString("Limit: unlimited*");
    } else {
        m_limitLabel->setString(fmt::format("Limit: {}", m_settings.objectLimit).c_str());
    }
}

void R387Popup::refreshPreview() {
    if (!m_previewGrid) return;
    m_previewGrid->removeAllChildrenWithCleanup(true);
    if (!m_image.valid()) {
        m_statsLabel->setString("Choose a PNG, JPG, or BMP image");
        return;
    }

    const auto plan = r387::makePlan(m_image, m_settings);
    if (!plan.valid()) {
        m_statsLabel->setString("Preview unavailable");
        return;
    }
    const float cw = kPreviewW / static_cast<float>(std::max(1, plan.columns));
    const float ch = kPreviewH / static_cast<float>(std::max(1, plan.rows));
    const float square = std::max(1.f, std::min(cw, ch));
    const float gridW = square * static_cast<float>(plan.columns);
    const float gridH = square * static_cast<float>(plan.rows);
    const float left = (kPreviewW - gridW) * 0.5f;
    const float bottom = (kPreviewH - gridH) * 0.5f;
    for (const auto& cell : plan.cells) {
        auto swatch = CCLayerColor::create({cell.color.r, cell.color.g, cell.color.b, 255}, square + 0.1f, square + 0.1f);
        swatch->setPosition({left + static_cast<float>(cell.x) * square,
                             bottom + static_cast<float>(plan.rows - 1 - cell.y) * square});
        m_previewGrid->addChild(swatch);
    }
    m_statsLabel->setString(fmt::format("{} objects | {} × {} cells", plan.cells.size(), plan.columns, plan.rows).c_str());
}

void R387Popup::onGenerate(CCObject*) {
    if (!m_editorUI || !m_editorUI->m_editorLayer) {
        showError("Editor unavailable", "Open a Geometry Dash level in the editor and try again.");
        return;
    }
    if (!m_image.valid()) {
        showError("Image required", "Choose a PNG, JPG, JPEG, or BMP image first.");
        return;
    }
    const auto selection = currentSelection(m_editorUI);
    if (selection.empty()) {
        showError("Select objects first", "Select one or more blocks/objects to define the placement area, then reopen R387 Agent.");
        return;
    }

    const auto plan = r387::makePlan(m_image, m_settings);
    if (!plan.valid()) {
        showError("Unable to generate", plan.error);
        return;
    }
    if (plan.cells.empty()) {
        showError("Nothing to draw", "The image is fully transparent at this resolution. Try another image or increase the object limit.");
        return;
    }

    float minX = selection.front()->getPositionX();
    float maxX = minX;
    float minY = selection.front()->getPositionY();
    float maxY = minY;
    for (auto* object : selection) {
        const auto p = object->getPosition();
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }
    // Treat the selection bounds as a placement reference. The original selection is not modified.
    const float boundsW = std::max(kTileSize, maxX - minX + kTileSize);
    const float boundsH = std::max(kTileSize, maxY - minY + kTileSize);
    const float cell = std::max(0.5f, std::min(boundsW / plan.columns, boundsH / plan.rows));
    const float imageW = cell * plan.columns;
    const float imageH = cell * plan.rows;
    const float startX = (minX + maxX) * 0.5f - imageW * 0.5f + cell * 0.5f;
    const float startY = (minY + maxY) * 0.5f + imageH * 0.5f - cell * 0.5f;

    std::size_t inserted = 0;
    for (const auto& pixel : plan.cells) {
        if (inserted >= plan.budget) break; // Final enforcement at the insertion boundary.
        auto object = GameObject::createWithKey(1);
        if (!object) continue;
        object->setPosition({startX + static_cast<float>(pixel.x) * cell,
                             startY - static_cast<float>(pixel.y) * cell});
        object->setScale(cell / kTileSize);
        object->setObjectColor({pixel.color.r, pixel.color.g, pixel.color.b});
        m_editorUI->addObject(object);
        ++inserted;
    }

    m_statusLabel->setString("Generation complete");
    FLAlertLayer::create(
        "R387 Agent complete",
        fmt::format("Inserted {} editable objects. The original selection was left in place.\n\nNote: this first version uses colored block objects, which may collide during gameplay.", inserted).c_str(),
        "OK"
    )->show();
}
