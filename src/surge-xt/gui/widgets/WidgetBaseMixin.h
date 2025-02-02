/*
** Surge Synthesizer is Free and Open Source Software
**
** Surge is made available under the Gnu General Public License, v3.0
** https://www.gnu.org/licenses/gpl-3.0.en.html
**
** Copyright 2004-2021 by various individuals as described by the Git transaction log
**
** All source at: https://github.com/surge-synthesizer/surge.git
**
** Surge was a commercial product from 2004-2018, with Copyright and ownership
** in that period held by Claes Johanson at Vember Audio. Claes made Surge
** open source in September 2018.
*/

#ifndef SURGE_XT_WIDGETBASEMIXIN_H
#define SURGE_XT_WIDGETBASEMIXIN_H

#include "SkinSupport.h"
#include "SurgeGUICallbackInterfaces.h"

#include "juce_gui_basics/juce_gui_basics.h"

#include <unordered_set>
#include "MainFrame.h"

#include "SurgeGUIEditor.h"

namespace Surge
{
namespace Widgets
{
template <typename T>
struct WidgetBaseMixin : public Surge::GUI::SkinConsumingComponent,
                         public Surge::GUI::IComponentTagValue
{
    WidgetBaseMixin() { asT()->setWantsKeyboardFocus(true); }
    inline T *asT() { return static_cast<T *>(this); }

    uint32_t tag{0};
    void setTag(uint32_t t) { tag = t; }
    uint32_t getTag() const override { return tag; }

    std::unordered_set<Surge::GUI::IComponentTagValue::Listener *> listeners;
    void addListener(Surge::GUI::IComponentTagValue::Listener *t) { listeners.insert(t); }
    void notifyValueChanged()
    {
        for (auto t : listeners)
            t->valueChanged(this);

        if (auto *handler = asT()->getAccessibilityHandler())
        {
            if (handler->getValueInterface())
            {
                handler->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
            }
        }
    }
    void notifyControlModifierClicked(const juce::ModifierKeys &k, bool addRMB = false)
    {
        auto kCopy = k;
        if (addRMB)
        {
            kCopy = juce::ModifierKeys(k.getRawFlags() | juce::ModifierKeys::rightButtonModifier);
        }
        for (auto t : listeners)
            t->controlModifierClicked(this, kCopy, false);
    }

    void notifyControlModifierDoubleClicked(const juce::ModifierKeys &k)
    {
        for (auto t : listeners)
            t->controlModifierClicked(this, k, true);
    }

    void notifyBeginEdit()
    {
        for (auto t : listeners)
            t->controlBeginEdit(this);
    }

    void notifyEndEdit()
    {
        for (auto t : listeners)
            t->controlEndEdit(this);
    }

    juce::Point<float> enqueueStartPosition{-18.f, -18.f};
    void enqueueFutureInfowindow(SurgeGUIEditor::InfoQAction place,
                                 const juce::Point<float> &fromPosition = juce::Point<float>{-17.f,
                                                                                             -17.f})
    {
        /*
         * So what the heck is this you may ask? Well when juce shows the info window on the
         * very first go round, since it is a hierarchy change, juce sends us a zero-distance
         * mouse moved event. So we need ot make sure, in the case of a start and only a start,
         * that if we get two in a row they are from different places. See #5487
         */
        if (place == SurgeGUIEditor::InfoQAction::START)
        {
            jassert(fromPosition.x != -17 && fromPosition.y != -17);
            if (enqueueStartPosition == fromPosition)
            {
                return;
            }
            enqueueStartPosition = fromPosition;
        }
        else
        {
            enqueueStartPosition = juce::Point<float>{-18.f, -18.f};
        }
        auto t = getTag();
        auto sge = firstListenerOfType<SurgeGUIEditor>();
        if (sge)
            sge->enqueueFutureInfowindow(t, asT()->getBounds(), place);
    }

    void showInfowindow(bool isEditingModulation)
    {
        auto l = asT()->getBounds();
        auto t = getTag();
        auto sge = firstListenerOfType<SurgeGUIEditor>();
        if (sge)
            sge->showInfowindow(t, l, isEditingModulation);
    }

    void showInfowindowSelfDismiss(bool isEditingModulation)
    {
        auto l = asT()->getBounds();
        auto t = getTag();
        auto sge = firstListenerOfType<SurgeGUIEditor>();
        if (sge)
            sge->showInfowindowSelfDismiss(t, l, isEditingModulation);
    }

    void updateInfowindowContents(bool isEditingModulation)
    {
        auto t = getTag();
        auto sge = firstListenerOfType<SurgeGUIEditor>();
        if (sge)
            sge->updateInfowindowContents(t, isEditingModulation);
    }
    void hideInfowindowNow()
    {
        auto sge = firstListenerOfType<SurgeGUIEditor>();
        if (sge)
            sge->hideInfowindowNow();
    }

    void hideInfowindowSoon()
    {
        auto sge = firstListenerOfType<SurgeGUIEditor>();
        if (sge)
            sge->hideInfowindowSoon();
    }

    template <typename U> U *firstListenerOfType()
    {
        for (auto u : listeners)
        {
            auto q = dynamic_cast<U *>(u);
            if (q)
                return q;
        }
        return nullptr;
    }

    bool forwardedMainFrameMouseDowns(const juce::MouseEvent &e)
    {
        if (e.mods.isMiddleButtonDown())
        {
            auto sge = firstListenerOfType<SurgeGUIEditor>();
            if (sge && sge->frame)
                sge->frame->mouseDown(e);
            return true;
        }
        return false;
    }

    bool supressMainFrameMouseEvent(const juce::MouseEvent &e)
    {
        return firstListenerOfType<SurgeGUIEditor>() && e.mods.isMiddleButtonDown();
    }
};
} // namespace Widgets
} // namespace Surge
#endif // SURGE_XT_WIDGETBASEMIXIN_H
