#ifndef N_GUITEXTVIEW_H
#define N_GUITEXTVIEW_H
//------------------------------------------------------------------------------
/**
    @class nGuiTextView
    @ingroup NebulaGuiSystem
    @brief A multiline text viewing widgets, which can display lines of
    text.
    
    This is the basis for more advanced widgets which need to display text
    in multiple lines.
    
    (C) 2004 RadonLabs GmbH
*/
#include "gui/nguiformlayout.h"
#include "gui/nguislider2.h"

//------------------------------------------------------------------------------
class nGuiTextView : public nGuiFormLayout
{
public:
    /// constructor
    nGuiTextView();
    /// destructor
    virtual ~nGuiTextView();
    /// set the font to use (see nGuiServer::AddFont())
    void SetFont(const char* fontName);
    /// get the font definition
    const char* GetFont() const;
    /// set text color
    void SetTextColor(const vector4& c);
    /// get text color
    const vector4& GetTextColor() const;
    /// set horizontal border
    void SetBorder(float b);
    /// get horizontal border
    float GetBorder() const;
    /// begin appending text
    void BeginAppend();
    /// append a line of text (copies the text)
    void AppendLine(const char* t);
    /// finish appending text
    void EndAppend();
    /// get line at index
    const char* GetLineAt(int i) const;
    /// set the line offset (the index of the first visible line)
    void SetLineOffset(int i);
    /// get the line offset 
    int GetLineOffset() const;
    /// enable/disable selection handling
    void SetSelectionEnabled(bool b);
    /// get selection handling flag
    bool GetSelectionEnabled() const;
    /// set selection index
    void SetSelectionIndex(int i);
    /// get the selection index
    int GetSelectionIndex() const;
    /// get the currently selected text, or 0 if nothing selected
    const char* GetSelection() const;
    /// get overall number of lines
    int GetNumLines() const;
    /// get the number of lines that would currently fit into the text view
    int GetNumVisibleLines();
    /// called when widget is becoming visible
    virtual void OnShow();
    /// called when widget is becoming invisible
    virtual void OnHide();
    /// called when widget position or size changes
    virtual void OnRectChange(const rectangle& newRect);   
    /// called per frame when parent widget is visible
    virtual void OnFrame();
    /// notify widget of an event
    virtual void OnEvent(const nGuiEvent& event);
    /// handle button down
    virtual bool OnButtonDown(const vector2& mousePos);
    /// rendering
    virtual bool Render();

protected:
    /// validate the font resource
    void ValidateFont();
    /// update the slider range values
    void UpdateSliderValues();
    /// update slider visibility status
    void UpdateSliderVisibility();

    nAutoRef<nGfxServer2> refGfxServer;
    nAutoRef<nResourceServer> refResourceServer;
    nRef<nFont2> refFont;
    nRef<nGuiSlider2> refSlider;

    nString fontName;
    float lineHeight;
    float border;
    vector4 textColor;
    nArray<nString> textArray;
    bool selectionEnabled;
    int lineOffset;
    int selectionIndex;
};

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::SetBorder(float b)
{
    this->border = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
float
nGuiTextView::GetBorder() const
{
    return this->border;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::SetSelectionEnabled(bool b)
{
    this->selectionEnabled = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
nGuiTextView::GetSelectionEnabled() const
{
    return this->selectionEnabled;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::SetSelectionIndex(int i)
{
    this->selectionIndex = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nGuiTextView::GetSelectionIndex() const
{
    return this->selectionIndex;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nGuiTextView::GetNumLines() const
{
    return this->textArray.Size();
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::SetLineOffset(int i)
{
    this->lineOffset = i;
    if (this->refSlider.isvalid())
    {
        this->refSlider->SetVisibleRangeStart(float(i));
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nGuiTextView::GetLineOffset() const
{
    return this->lineOffset;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::BeginAppend()
{
    this->textArray.Clear();
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::AppendLine(const char* t)
{
    n_assert(t);
    this->textArray.Append(t);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::UpdateSliderValues()
{
    if (this->refSlider.isvalid())
    {
        this->refSlider->SetRangeSize(float(this->textArray.Size()));
        this->refSlider->SetVisibleRangeSize(float(this->GetNumVisibleLines()));
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::EndAppend()
{
    this->UpdateSliderValues();
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::SetTextColor(const vector4& c)
{
    this->textColor = c;
}

//------------------------------------------------------------------------------
/**
*/
inline
const vector4&
nGuiTextView::GetTextColor() const
{
    return this->textColor;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nGuiTextView::SetFont(const char* f)
{
    this->fontName = f;
}

//------------------------------------------------------------------------------
/**
*/
inline
const char*
nGuiTextView::GetFont() const
{
    return this->fontName.Get();
}

//------------------------------------------------------------------------------
/**
*/
inline
const char*
nGuiTextView::GetLineAt(int i) const
{
    return this->textArray[i].Get();
}

//------------------------------------------------------------------------------
/**
    Returns a pointer to the selected text, or 0 if no valid selection
    exists.
*/
inline
const char*
nGuiTextView::GetSelection() const
{
    int index = this->selectionIndex;
    if ((index >= 0) && (index < this->textArray.Size()))
    {
        return this->textArray[index].Get();
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
#endif    
