#include "InputFilter.h"
#include "../CurvePreset/CurvePreset.h"
#include "../../Brain.h"

InputFilterManager::InputFilterManager(String name) :
    BaseManager(name)
{
    itemDataType = "InputFilter";
    selectItemWhenCreated = false;
}

InputFilter::InputFilter(var params) :
    BaseItem(params.getProperty("name", "Input filter"))
{
    saveAndLoadRecursiveData = true;
    itemDataType = "InputFilter";

    type = addEnumParameter("Type", "What this filter does to the input");
    type->addOption("Damping", "damping");
    type->addOption("Ease / lerp", "ease");
    type->addOption("Log scale", "log");
    type->addOption("Curve preset", "curve");

    dampMode = addEnumParameter("Mode", "Exponential: ease-out, about 95% of the way after Time. Constant rate: fixed speed, full travel in Time");
    dampMode->addOption("Exponential", "exp");
    dampMode->addOption("Constant rate", "rate");

    easeShape = addEnumParameter("Easing", "Shape of the ease");
    easeShape->addOption("Linear", "linear");
    easeShape->addOption("Sine", "sine");
    easeShape->addOption("Quad", "quad");
    easeShape->addOption("Cubic", "cubic");
    easeShape->addOption("Expo", "expo");

    easeDir = addEnumParameter("Direction", "Ease in, out or both");
    easeDir->addOption("In-Out", "inout");
    easeDir->addOption("In", "in");
    easeDir->addOption("Out", "out");

    time = addFloatParameter("Time", "Seconds", 0.5f, 0);
    timeDown = addFloatParameter("Time down", "Seconds when the value falls, 0 = same as Time", 0, 0);

    logShape = addEnumParameter("Shape", "Log: fast start. Exp: slow start");
    logShape->addOption("Exp (slow start)", "exp");
    logShape->addOption("Log (fast start)", "log");
    amount = addFloatParameter("Amount", "Strength of the taper, 1 = linear", 10, 1);

    presetId = addIntParameter("Curve preset ID", "ID of the curve preset used as the input to output map", 1, 1);

    updateDisplay();
}

InputFilter::~InputFilter() {}

void InputFilter::onContainerParameterChangedInternal(Parameter* p)
{
    BaseItem::onContainerParameterChangedInternal(p);
    if (p == type || p == dampMode) updateDisplay();
    if (p == type || p == dampMode || p == time) reset();
}

void InputFilter::updateDisplay()
{
    String t = type->getValue().toString();
    bool damp = t == "damping";
    bool ease = t == "ease";
    dampMode->hideInEditor = !damp;
    timeDown->hideInEditor = !damp;
    easeShape->hideInEditor = !ease;
    easeDir->hideInEditor = !ease;
    time->hideInEditor = !(damp || ease);
    logShape->hideInEditor = t != "log";
    amount->hideInEditor = t != "log";
    presetId->hideInEditor = t != "curve";
    queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}

float InputFilter::ease(float p) const
{
    p = jlimit(0.0f, 1.0f, p);
    String s = easeShape->getValue().toString();
    String d = easeDir->getValue().toString();
    auto in = [&](float x) {
        if (s == "sine") return 1.0f - std::cos(x * MathConstants<float>::halfPi);
        if (s == "quad") return x * x;
        if (s == "cubic") return x * x * x;
        if (s == "expo") return x <= 0 ? 0.0f : std::pow(2.0f, 10.0f * (x - 1.0f));
        return x;
    };
    auto out = [&](float x) { return 1.0f - in(1.0f - x); };
    float r;
    if (d == "in") r = in(p);
    else if (d == "out") r = out(p);
    else r = p < 0.5f ? 0.5f * in(p * 2.0f) : 1.0f - 0.5f * in((1.0f - p) * 2.0f);
    return jlimit(0.0f, 1.0f, r);
}

float InputFilter::process(float in, double dt)
{
    String t = type->getValue().toString();

    if (t == "log") {
        float b = amount->floatValue();
        if (b <= 1.0001f) return in;
        in = jlimit(0.0f, 1.0f, in);
        if (logShape->getValue().toString() == "log") return std::log(1.0f + (b - 1.0f) * in) / std::log(b);
        return (std::pow(b, in) - 1.0f) / (b - 1.0f);
    }

    if (t == "curve") {
        CurvePreset* p = Brain::getInstance()->getCurvePresetById(presetId->intValue());
        if (p == nullptr) return in;
        return jlimit(0.0f, 1.0f, (float)p->curve.getValueAtPosition(jlimit(0.0f, 1.0f, in)));
    }

    if (!initialised) {
        initialised = true;
        y = lastIn = easeStart = in;
        easeT = time->floatValue();
        return y;
    }

    if (t == "damping") {
        lastIn = in;
        float tm = time->floatValue();
        if (in < y && timeDown->floatValue() > 0) tm = timeDown->floatValue();
        if (tm <= 0) { y = in; return y; }
        if (dampMode->getValue().toString() == "rate") {
            float step = (float)(dt / tm);
            y = std::abs(in - y) <= step ? in : y + (in > y ? step : -step);
        }
        else {
            float tau = tm / 3.0f;
            y += (in - y) * (1.0f - (float)std::exp(-dt / tau));
            if (std::abs(in - y) < 0.0005f) y = in;
        }
        return y;
    }

    // ease
    float tm = time->floatValue();
    if (std::abs(in - lastIn) > 0.0005f) {
        easeStart = y;
        lastIn = in;
        easeT = 0;
    }
    if (tm <= 0) { y = lastIn; easeT = 0; return y; }
    easeT += dt;
    if (easeT >= tm) { y = lastIn; return y; }
    y = easeStart + (lastIn - easeStart) * ease((float)(easeT / tm));
    return y;
}

bool InputFilter::isSettled() const
{
    String t = type->getValue().toString();
    if (t == "log" || t == "curve" || !initialised) return true;
    if (t == "damping") return y == lastIn;
    return y == lastIn && easeT >= time->floatValue();
}
