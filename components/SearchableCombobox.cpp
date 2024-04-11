#include "SearchableCombobox.h"

SearchableCombobox::SearchableCombobox() : juce::ComboBox("searchable combobox")
{
	setEditableText(true);
	setWantsKeyboardFocus(true);
}

bool SearchableCombobox::keyStateChanged(bool x)
{
	if (x) {
		return true;
	}
	int i = 0;
	if (!isPopupActive()) {
		showEditor();
		setWantsKeyboardFocus(true);
	}
	return true;
}
