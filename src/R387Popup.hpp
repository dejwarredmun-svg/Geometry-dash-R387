#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

#include "ImageProcessor.hpp"
#include <filesystem>
#include <string>

namespace r387 { class R387Popup; }

class R387Popup final : public geode::Popup {
public:
    static R387Popup* create(EditorUI* editorUI);

protected:
    bool init(EditorUI* editorUI);

private:
    void onChooseImage(cocos2d::CCObject*);
    void onLimitDown(cocos2d::CCObject*);
    void onLimitUp(cocos2d::CCObject*);
    void onUnlimited(cocos2d::CCObject*);
    void onGenerate(cocos2d::CCObject*);
    void refreshPreview();
    void refreshLabels();
    void showError(const std::string& title, const std::string& detail);

    EditorUI* m_editorUI = nullptr;
    r387::Raster m_image;
    std::filesystem::path m_imagePath;
    r387::Settings m_settings;
    cocos2d::CCLabelBMFont* m_imageLabel = nullptr;
    cocos2d::CCLabelBMFont* m_selectionLabel = nullptr;
    cocos2d::CCLabelBMFont* m_limitLabel = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
    cocos2d::CCLabelBMFont* m_statsLabel = nullptr;
    cocos2d::CCNode* m_previewGrid = nullptr;
};
