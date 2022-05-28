#pragma once

class TreeConfig
{
public:
	TreeConfig();
	void setCompact(bool compact) { compact_ = compact; }
	bool compact() const{ return compact_; }
	void setTabSize(int tabSize) { tabSize_ = tabSize; }
	int tabSize() const { return tabSize_; }
	void setFullRowMode(bool fullRowMode) { fullRowMode_ = fullRowMode; }
	bool fullRowMode() const { return fullRowMode_; }
	void setFullRowContainres(bool fullRowContainers) { fullRowContainers_ = fullRowContainers; }
	bool fullRowContainers() const { return fullRowContainers_; }
	void setHideUntranslated(bool hideUntranslated) { hideUntranslated_ = hideUntranslated; }
	bool hideUntranslated() const{ return hideUntranslated_; }
	void setValueColumnWidth(float valueColumnWidth) { valueColumnWidth_ = valueColumnWidth; }
	float valueColumnWidth() const { return valueColumnWidth_; }
	void setImmediateUpdate(bool immediateUpdate) { immediateUpdate_ = immediateUpdate; }
	bool immediateUpdate() const{ return immediateUpdate_; }
	void setFilter(int filter) { filter_ = filter; }
	void setFilterWhenType(bool filterWhenType) {	filterWhenType_ = filterWhenType; }
	void setExpandLevels(int levels) { expandLevels_ = levels; }
	void setUndoEnabled(bool enabled, bool full = false) { undoEnabled_ = enabled; fullUndo_ = full; }
	void setShowContainerIndices(bool showContainerIndices) { showContainerIndices_ = showContainerIndices; }
	void setSliderUpdateDelay(int delayMS) { sliderUpdateDelay_ = delayMS; }
	bool showContainerIndices() const{ return showContainerIndices_; }
	int _defaultRowHeight() const { return defaultRowHeight_; }

	static TreeConfig defaultConfig;

protected:
	bool compact_;
	bool fullRowMode_;
	bool fullRowContainers_;
	bool immediateUpdate_;
	bool hideUntranslated_;
	bool showContainerIndices_;
	bool filterWhenType_;
	float valueColumnWidth_;
	int filter_;
	int tabSize_;
	int sliderUpdateDelay_;
	int defaultRowHeight_;

	int expandLevels_;
	bool undoEnabled_;
	bool fullUndo_;
};
