/*
  ==============================================================================

    TempoManagerUI.h

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "Tempo.h"

class TempoManagerUI :
	public BaseManagerShapeShifterUI<BaseManager<Tempo>, Tempo, BaseItemUI<Tempo>>
{
public:
	TempoManagerUI(const String &contentName);
	~TempoManagerUI();

	static TempoManagerUI * create(const String &name) { return new TempoManagerUI(name); }
};
