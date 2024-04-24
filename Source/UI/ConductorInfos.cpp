#include <JuceHeader.h>
#include "ConductorInfos.h"
#include "Brain.h"

//==============================================================================
ConductorInfosUI::ConductorInfosUI(const String& contentName):
    ShapeShifterContent(ConductorInfos::getInstance(), contentName)
{
    
}

ConductorInfosUI::~ConductorInfosUI()
{
}

juce_ImplementSingleton(ConductorInfos);

ConductorInfos::ConductorInfos()
{
    addAndMakeVisible(displayBtn);
    addAndMakeVisible(currentCueName);
    addAndMakeVisible(currentCueId);
    addAndMakeVisible(currentCueText);
    addAndMakeVisible(nextCueGo);
    addAndMakeVisible(nextCueName);
    addAndMakeVisible(commands);

    addAndMakeVisible(inspectPrevBtn);
    addAndMakeVisible(inspectCurrBtn);
    addAndMakeVisible(inspectNextBtn);
    addAndMakeVisible(inspectCuelistBtn);

    currentCueName.addListener(this);
    currentCueId.addListener(this);
    currentCueText.addListener(this);
    nextCueGo.addListener(this);
    nextCueName.addListener(this);

    displayBtn.onClick = [this](){
        displayMode = (displayMode +1) % 3;
        updateDisplayBtn();
        resized();
    };

    currentCueText.onEditorShow = [this](){
        currentCueText.getCurrentTextEditor()->setCaretVisible(true);
        currentCueText.getCurrentTextEditor()->setMultiLine(true);
        currentCueText.getCurrentTextEditor()->setShiftReturnKeyStartsNewLine(true);
        };

    inspectPrevBtn.onClick = [this]() {inspect(-1); };
    inspectCurrBtn.onClick = [this]() {inspect(0); };
    inspectNextBtn.onClick = [this]() {inspect(1); };
    inspectCuelistBtn.onClick = [this]() {inspectCuelist(); };

    addAndMakeVisible(upLabel);
    addAndMakeVisible(downLabel);
    addAndMakeVisible(ltpLabel);
    addAndMakeVisible(fadeLabel);
    addAndMakeVisible(delayLabel);

    displayBtn.setButtonText("Text");
    inspectPrevBtn.setButtonText("Inspect Prev.");
    inspectCurrBtn.setButtonText("Inspect Curr.");
    inspectNextBtn.setButtonText("Inspect Next");
    inspectCuelistBtn.setButtonText("Inspect Cuelist");

    currentCueName.setText("", juce::NotificationType::dontSendNotification);
    currentCueId.setText("", juce::NotificationType::dontSendNotification);
    currentCueText.setText("", juce::NotificationType::dontSendNotification);
    nextCueGo.setText("", juce::NotificationType::dontSendNotification);
    nextCueName.setText("", juce::NotificationType::dontSendNotification);
    upLabel.setText("up", juce::NotificationType::dontSendNotification);
    downLabel.setText("down", juce::NotificationType::dontSendNotification);
    ltpLabel.setText("ltp", juce::NotificationType::dontSendNotification);
    fadeLabel.setText("fade", juce::NotificationType::dontSendNotification);
    delayLabel.setText("delay", juce::NotificationType::dontSendNotification);

    currentCueName.setJustificationType(juce::Justification::centred);
    currentCueId.setJustificationType(juce::Justification::centred);
    currentCueText.setJustificationType(juce::Justification::centred);
    nextCueGo.setJustificationType(juce::Justification::centred);
    nextCueName.setJustificationType(juce::Justification::centred);
    commands.setJustificationType(juce::Justification::centred);

    upLabel.setJustificationType(juce::Justification::centredRight);
    downLabel.setJustificationType(juce::Justification::centredRight);
    ltpLabel.setJustificationType(juce::Justification::centred);
    fadeLabel.setJustificationType(juce::Justification::centred);
    delayLabel.setJustificationType(juce::Justification::centred);

    currentCueName.setEditable(false, true, false);
    currentCueText.setEditable(false, true, false);
    nextCueGo.setEditable(false, true, false);
    nextCueName.setEditable(false, true, false);

}

ConductorInfos::~ConductorInfos()
{
    if (currentFade != nullptr) {
        removeChildComponent(currentFade);
        delete currentFade;
    }
    if (nextHTPInDelay != nullptr) { removeChildComponent(nextHTPInDelay); delete nextHTPInDelay; }
    nextHTPInDelay = nullptr;
    if (nextHTPOutDelay != nullptr) { removeChildComponent(nextHTPOutDelay); delete nextHTPOutDelay; }
    nextHTPOutDelay = nullptr;
    if (nextLTPDelay != nullptr) { removeChildComponent(nextLTPDelay); delete nextLTPDelay; }
    nextLTPDelay = nullptr;
    if (nextHTPInFade != nullptr) { removeChildComponent(nextHTPInFade); delete nextHTPInFade; }
    nextHTPInFade = nullptr;
    if (nextHTPOutFade != nullptr) { removeChildComponent(nextHTPOutFade); delete nextHTPOutFade; }
    nextHTPOutFade = nullptr;
    if (nextLTPFade != nullptr) { removeChildComponent(nextLTPFade); delete nextLTPFade; }
    nextLTPFade = nullptr;

}

void ConductorInfos::paint (juce::Graphics& g)
{
    if (engine == nullptr) { return; }
    if (engine != nullptr) { return; }
    g.fillAll(juce::Colours::black);   // clear the background

    int targetCueId = engine->conductorCuelistId->intValue();
    Cuelist* target = Brain::getInstance()->getCuelistById(targetCueId);
    if (target == nullptr) {
        g.drawText("Cuelist "+String(targetCueId)+" doesn't exist", getLocalBounds(), juce::Justification::centred, true);
        return;
    }

    Cue* nextCue = target->getNextCue();

    float textSize = engine->conductorTextSize->floatValue();
    float titleSize = engine->conductorTitleSize->floatValue();

    float w = getLocalBounds().getWidth();
    float h = getLocalBounds().getHeight() - 20;

    float currentCueHeight = titleSize +10;
    float nextCueHeight = textSize +10;
    float nextCueGoHeight = titleSize +10;
    float currentCueTextHeight = h - currentCueHeight - nextCueHeight - nextCueGoHeight;

    if (currentCueTextHeight < currentCueHeight) {
        currentCueHeight = h/4;
        nextCueHeight = h/4;
        nextCueGoHeight = h/4;
        currentCueTextHeight = h/4;
    }

    float timingHeight = nextCueHeight + nextCueGoHeight;
    float timingElementSize = timingHeight / 3;
    float timingWidth = w/3;

    g.setColour (engine->conductorCurrentCueColor->getColor());

    g.setFont (titleSize);
    String currId = "";
    if (target->conductorCurrentCueId->floatValue() >= 0) {
        currId = String(target->conductorCurrentCueId->floatValue()) + " - ";
    }
    g.drawFittedText(currId+ target->conductorCurrentCueName->stringValue(), 0, 0, w, currentCueHeight, juce::Justification::centredTop, true);

    g.setFont(textSize);
    g.drawFittedText(target->conductorCurrentCueText->stringValue(), 0, currentCueHeight, w, currentCueTextHeight, juce::Justification::centredTop, true);

    g.setColour(engine->conductorNextCueColor->getColor());

    float topNext = h - nextCueHeight - nextCueGoHeight;

    g.setFont(titleSize);
    g.drawFittedText(target->conductorNextCueGo->stringValue(), 0, topNext, w-timingWidth, nextCueGoHeight, juce::Justification::centredTop, true);

    g.setFont(textSize);
    String nextId = "";
    if (target->conductorNextCueId->floatValue() >= 0) {
        nextId = String(target->conductorNextCueId->floatValue()) + " - ";
    }
    g.drawFittedText(nextId + target->conductorNextCueName->stringValue(), 0, h - nextCueHeight, w-timingWidth, nextCueHeight, juce::Justification::centredTop, true);

    if (nextCue != nullptr) {

        int timeW = (w / 3) / 4;
        int timeH = timingHeight / 3;

        g.setFont(timingElementSize*0.5);
        g.drawFittedText("Up", w - (3 * timeW), h-(3*timeH), timeW, timeH, juce::Justification::centred, true);
        g.drawFittedText("Down", w - (2 * timeW), h-(3*timeH), timeW, timeH, juce::Justification::centred, true);
        g.drawFittedText("LTP", w - (1 * timeW), h-(3*timeH), timeW, timeH, juce::Justification::centred, true);
        //int r = Random::getSystemRandom().nextInt(100);

        g.drawFittedText("Delay", w - (4 * timeW), h - (2 * timeH), timeW, timeH, juce::Justification::centred, true);
        g.drawFittedText("Fade", w - (4 * timeW), h - (1 * timeH), timeW, timeH, juce::Justification::centred, true);
    }


}

void ConductorInfos::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    float textSize = engine->conductorTextSize->floatValue();
    float titleSize = engine->conductorTitleSize->floatValue();

    float w = getLocalBounds().getWidth();
    float h = getLocalBounds().getHeight() - 40;

    float currentCueHeight = titleSize + 20;
    float nextCueHeight = textSize + 20;
    float nextCueGoHeight = titleSize + 20;
    float currentCueTextHeight = h - currentCueHeight - nextCueHeight - nextCueGoHeight;

    if (currentCueTextHeight < currentCueHeight) {
        currentCueHeight = h / 4;
        nextCueHeight = h / 4;
        nextCueGoHeight = h / 4; 
        currentCueTextHeight = h / 4;
    }

    float timingHeight = nextCueHeight + nextCueGoHeight;

    int timeW = (w/3)/4;
    int timeH = timingHeight / 3;

    int idW = titleSize*3;

    displayBtn.setBounds(w * 0 / 5, 0, w / 5, 20);
    inspectPrevBtn.setBounds(w * 1 / 5, 0, w / 5, 20);
    inspectCurrBtn.setBounds(w * 2 / 5, 0, w / 5, 20);
    inspectNextBtn.setBounds(w * 3 / 5, 0, w / 5, 20);
    inspectCuelistBtn.setBounds(w * 4 / 5, 0, w / 5, 20);

    currentCueId.setBounds(0, 20, idW, floor(currentCueHeight));
    currentCueName.setBounds(idW, 20, w-idW, floor(currentCueHeight));
    nextCueGo.setBounds(0, 20+floor(currentCueHeight + currentCueTextHeight), floor(2 * w/3), floor(nextCueHeight));
    nextCueName.setBounds(0, floor(h - nextCueHeight), floor(2 * w/3), floor(nextCueHeight));

    if (displayMode == 0) {
        currentCueText.setVisible(true); 
        currentCueText.setBounds(0, 20+floor(currentCueHeight), floor(w), floor(currentCueTextHeight));
        commands.setVisible(false); 
        commands.setBounds(0, 0, 0, 0);
    }
    else if (displayMode == 1) {
        currentCueText.setVisible(false); 
        currentCueText.setBounds(0, 0, 0, 0);
        commands.setVisible(true); 
        commands.setBounds(0, 20+floor(currentCueHeight), floor(w), floor(currentCueTextHeight));
    }
    else if (displayMode == 2) {
        currentCueText.setVisible(true); 
        currentCueText.setBounds(0, 20+floor(currentCueHeight), floor(w/2), floor(currentCueTextHeight));
        commands.setVisible(true); 
        commands.setBounds(floor(w / 2), 20+floor(currentCueHeight), floor(w/2), floor(currentCueTextHeight));
    }


    upLabel.setBounds(w - (3 * timeW), h - (3 * timeH), timeW, timeH);
    downLabel.setBounds(w - (2 * timeW), h - (3 * timeH), timeW, timeH);
    ltpLabel.setBounds(w - (1 * timeW), h - (3 * timeH), timeW, timeH);
    delayLabel.setBounds(w - (4 * timeW), h - (2 * timeH), timeW, timeH);
    fadeLabel.setBounds(w - (4 * timeW), h - (1 * timeH), timeW, timeH);


    if (currentFade != nullptr) { currentFade->setBounds(0, h , w, 20); }
    if (nextHTPInDelay != nullptr) { nextHTPInDelay->setBounds(w - (3 * timeW), h - (2 * timeH), timeW, timeH); }
    if (nextHTPOutDelay != nullptr) { nextHTPOutDelay->setBounds(w - (2 * timeW), h - (2 * timeH), timeW, timeH); }
    if (nextLTPDelay != nullptr) { nextLTPDelay->setBounds(w - (1 * timeW), h - (2 * timeH), timeW, timeH); }
    if (nextHTPInFade != nullptr) { nextHTPInFade->setBounds(w - (3 * timeW), h - (1 * timeH), timeW, timeH); }
    if (nextHTPOutFade != nullptr) { nextHTPOutFade->setBounds(w - (2 * timeW), h - (1 * timeH), timeW, timeH); }
    if (nextLTPFade != nullptr) { nextLTPFade->setBounds(w - (1 * timeW), h - (1 * timeH), timeW, timeH); }
}

void ConductorInfos::updateStyle()
{
    Colour curr = engine->conductorCurrentCueColor->getColor();
    Colour next = engine->conductorNextCueColor->getColor();
    currentCueId.setColour(juce::Label::ColourIds::textColourId, curr);
    currentCueName.setColour(juce::Label::ColourIds::textColourId, curr);
    currentCueText.setColour(juce::Label::ColourIds::textColourId, curr);
    commands.setColour(juce::Label::ColourIds::textColourId, next);
    nextCueName.setColour(juce::Label::ColourIds::textColourId, next);
    nextCueGo.setColour(juce::Label::ColourIds::textColourId, next);
    upLabel.setColour(juce::Label::ColourIds::textColourId, next);
    downLabel.setColour(juce::Label::ColourIds::textColourId, next);
    ltpLabel.setColour(juce::Label::ColourIds::textColourId, next);
    delayLabel.setColour(juce::Label::ColourIds::textColourId, next);
    fadeLabel.setColour(juce::Label::ColourIds::textColourId, next);

    float textSize = engine->conductorTextSize->floatValue();
    float titleSize = engine->conductorTitleSize->floatValue();

    currentCueId.setFont(Font(titleSize, 0));
    currentCueName.setFont(Font(titleSize, 0));
    currentCueText.setFont(Font(textSize, 0));
    nextCueGo.setFont(Font(titleSize, 0));
    nextCueName.setFont(Font(textSize, 0));
    commands.setFont(Font(textSize, 0));


}

void ConductorInfos::linkFadeSlider()
{
    if (currentFade != nullptr) {
        removeChildComponent(currentFade);
        delete currentFade;
    }
    currentFade = nullptr;

    int targetCueId = engine->conductorCuelistId->intValue();
    Cuelist* target = Brain::getInstance()->getCuelistById(targetCueId);
    if (target == nullptr) {
        return;
    }

    currentFade = target->currentFade->createSlider();
    currentFade->showLabel = false;
    currentFade->showValue = false;
    currentFade->customFGColor = Colour(96, 96, 96);
    currentFade->customBGColor = Colour(32, 32, 32);
    currentFade->useCustomFGColor = true;
    currentFade->useCustomBGColor = true;

    addAndMakeVisible(currentFade);
    resized();
}

void ConductorInfos::updateContent()
{
    if (nextHTPInDelay != nullptr) { removeChildComponent(nextHTPInDelay); delete nextHTPInDelay; }
    nextHTPInDelay = nullptr;
    if (nextHTPOutDelay != nullptr) { removeChildComponent(nextHTPOutDelay); delete nextHTPOutDelay; }
    nextHTPOutDelay = nullptr;
    if (nextLTPDelay != nullptr) { removeChildComponent(nextLTPDelay); delete nextLTPDelay; }
    nextLTPDelay = nullptr;
    if (nextHTPInFade != nullptr) { removeChildComponent(nextHTPInFade); delete nextHTPInFade; }
    nextHTPInFade = nullptr;
    if (nextHTPOutFade != nullptr) { removeChildComponent(nextHTPOutFade); delete nextHTPOutFade; }
    nextHTPOutFade = nullptr;
    if (nextLTPFade != nullptr) { removeChildComponent(nextLTPFade); delete nextLTPFade; }
    nextLTPFade = nullptr;

    int targetCueId = engine->conductorCuelistId->intValue();
    Cuelist* target = Brain::getInstance()->getCuelistById(targetCueId);
    if (target == nullptr) {return;}
    Cue* currentCue = target->cueA;
    Cue* nextCue = target->getNextCue();

    if (currentCue != nullptr) {
        currentCueId.setText(currentCue->id->stringValue(), juce::NotificationType::dontSendNotification);
        currentCueName.setText(currentCue->niceName, juce::NotificationType::dontSendNotification);
        currentCueText.setText(currentCue->cueText->stringValue(), juce::NotificationType::dontSendNotification);
    }

    if (nextCue == nullptr) { return; }

    nextCueGo.setText(nextCue->goText->stringValue(), juce::NotificationType::dontSendNotification);
    nextCueName.setText(nextCue->niceName, juce::NotificationType::dontSendNotification);
    commands.setText("Next cue : \n" + nextCue->getCommandsText(true), juce::NotificationType::dontSendNotification);

    nextHTPInDelay = nextCue->htpInDelay->createSlider();
    nextHTPOutDelay = nextCue->htpOutDelay->createSlider();
    nextLTPDelay = nextCue->ltpDelay->createSlider();
    nextHTPInFade = nextCue->htpInFade->createSlider();
    nextHTPOutFade = nextCue->htpOutFade->createSlider();
    nextLTPFade = nextCue->ltpFade->createSlider();

    nextHTPInDelay->showLabel = false;
    nextHTPOutDelay->showLabel = false;
    nextLTPDelay->showLabel = false;
    nextHTPInFade->showLabel = false;
    nextHTPOutFade->showLabel = false;
    nextLTPFade->showLabel = false;

    nextHTPInDelay->customScaleFactor = 0.000000001;
    nextHTPOutDelay->customScaleFactor = 0.000000001;
    nextLTPDelay->customScaleFactor = 0.000000001;
    nextHTPInFade->customScaleFactor = 0.000000001;
    nextHTPOutFade->customScaleFactor = 0.000000001;
    nextLTPFade->customScaleFactor = 0.000000001;

    addAndMakeVisible(nextHTPInDelay);
    addAndMakeVisible(nextHTPOutDelay);
    addAndMakeVisible(nextLTPDelay);
    addAndMakeVisible(nextHTPInFade);
    addAndMakeVisible(nextHTPOutFade);
    addAndMakeVisible(nextLTPFade);
    resized();
}

void ConductorInfos::updateDisplayBtn()
{
    if (displayMode == 0) { displayBtn.setButtonText("Text"); }
    else if (displayMode == 1) { displayBtn.setButtonText("Commands"); }
    else if (displayMode == 2) { displayBtn.setButtonText("Both"); }
}

void ConductorInfos::labelTextChanged(Label* l)
{
    int targetCueId = engine->conductorCuelistId->intValue();
    Cuelist* target = Brain::getInstance()->getCuelistById(targetCueId);
    if (target == nullptr) { return; }
    Cue* currentCue = target->cueA;
    Cue* nextCue = target->getNextCue();

    if (nextCue != nullptr && l == &nextCueName)  { nextCue->setNiceName(l->getText()); }
    if (nextCue != nullptr && l == &nextCueGo) { nextCue->goText->setValue(l->getText()); }
    if (currentCue != nullptr && l == &currentCueName) { currentCue->setNiceName(l->getText()); }
    if (currentCue != nullptr && l == &currentCueText) { currentCue->cueText->setValue(l->getText()); }
}

void ConductorInfos::inspect(int delta)
{
    int targetCueId = engine->conductorCuelistId->intValue();
    Cuelist* target = Brain::getInstance()->getCuelistById(targetCueId);
    if (target == nullptr) { return; }
    Cue* currentCue = target->cueA;
    int index = -1;
    if (currentCue != nullptr) {
        index = target->cues.items.indexOf(currentCue);
    }
    index += delta;
    if (index >= 0 && index < target->cues.items.size()) {
        target->cues.items[index]->selectThis();
    }
}

void ConductorInfos::inspectCuelist()
{
    int targetCueId = engine->conductorCuelistId->intValue();
    Cuelist* target = Brain::getInstance()->getCuelistById(targetCueId);
    if (target == nullptr) { return; }
    target->selectThis();
}
