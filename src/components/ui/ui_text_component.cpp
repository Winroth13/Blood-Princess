#include "ui_text_component.hpp"

void UITextComponent::OnAlignHorizontalChanged() {
    if (this->alignHorizontal_ == "LEFT")
        this->alignHorizontal = UI::AlignHorizontal::LEFT;
    else if (this->alignHorizontal_ == "CENTRE")
        this->alignHorizontal = UI::AlignHorizontal::CENTRE;
    else if (this->alignHorizontal_ == "RIGHT")
        this->alignHorizontal = UI::AlignHorizontal::RIGHT;
}

void UITextComponent::OnAlignVerticalChanged() {
    if (this->alignVertical_ == "TOP")
        this->alignVertical = UI::AlignVertical::TOP;
    else if (this->alignVertical_ == "CENTRE")
        this->alignVertical = UI::AlignVertical::CENTRE;
    else if (this->alignVertical_ == "BOTTOM")
        this->alignVertical = UI::AlignVertical::BOTTOM;
}

void UITextComponent::SetAlignHorizontal(UI::AlignHorizontal alignHorizontal) {
    this->alignHorizontal = alignHorizontal;

    if (alignHorizontal == UI::AlignHorizontal::LEFT)
        this->alignHorizontal_ = "LEFT";
    else if (alignHorizontal == UI::AlignHorizontal::CENTRE)
        this->alignHorizontal_ = "CENTRE";
    else if (alignHorizontal == UI::AlignHorizontal::RIGHT)
        this->alignHorizontal_ = "RIGHT";
}

void UITextComponent::SetAlignVertical(UI::AlignVertical alignVertical) {
    this->alignVertical = alignVertical;

    if (alignVertical == UI::AlignVertical::TOP)
        this->alignVertical_ = "TOP";
    else if (alignVertical == UI::AlignVertical::CENTRE)
        this->alignVertical_ = "CENTRE";
    else if (alignVertical == UI::AlignVertical::BOTTOM)
        this->alignVertical_ = "BOTTOM";
}