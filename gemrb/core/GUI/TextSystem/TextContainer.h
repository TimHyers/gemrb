/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2014 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TEXTCONTAINER_H
#define TEXTCONTAINER_H

#include "Font.h"
#include "GUI/View.h"
#include "Region.h"
#include "System/String.h"

#include <deque>

namespace GemRB {

class Font;
class Palette;
class Sprite2D;
class ContentContainer;
class TextContainer;

// interface for both content and content containers
class Content {
friend class ContentContainer;
protected:
	// content doesnt have an x, y (-1, -1) unless we want absolute positioning
	// same applies for the dimensions, 0x0 implies unlimited area
	Region frame; // TODO: origin currently unused
	ContentContainer* parent;

public:
	Content(const Size& size);
	virtual ~Content();

	virtual Size ContentFrame() const { return frame.Dimensions(); };

protected:
	// point is relative to Region. Region is a screen region.
	virtual void DrawContentsInRegions(const Regions&, const Point&) const=0;
	virtual Regions LayoutForPointInRegion(Point p, const Region&) const;
};


// Content classes
class TextSpan : public Content
{
private:
	String text;
	const Font* font;
	Holder<Palette> palette;

public:
	// make a "block" of text that always occupies the area of "size", or autosizes if size in NULL
	// TODO: we should probably be able to align the text in the frame
	TextSpan(const String& string, const Font* font, Holder<Palette> pal = NULL, const Size* = NULL);

	const String& Text() const { return text; };

protected:
	virtual void DrawContentsInRegions(const Regions&, const Point&) const;
	virtual Regions LayoutForPointInRegion(Point p, const Region&) const;

private:
	inline const Font* LayoutFont() const;
	inline Region LayoutInFrameAtPoint(const Point&, const Region&) const;
};


class ImageSpan : public Content
{
private:
	Sprite2D* image;

public:
	ImageSpan(Sprite2D* image);

protected:
	virtual void DrawContentsInRegions(const Regions&, const Point&) const;
};


// Content container classes
class ContentContainer : public View
{
public:
	typedef std::list<Content*> ContentList;
	
protected:
	ContentList contents;

	struct Layout {
		const Content* content;
		Regions regions;

		Layout(const Content* c, const Regions r)
		: content(c), regions(r) {}

		bool operator==(const Content* c) const {
			return c == content;
		}

		bool operator<(const Point& p) const {
			const Region& r = regions.back();
			return r.y < p.y || (r.x < p.x && r.y == p.y);
		}

		bool PointInside(const Point& p) const {
			Regions::const_iterator rit = regions.begin();
			for (; rit != regions.end(); ++rit) {
				if ((*rit).PointInside(p)) {
					return true;
				}
			}
			return false;
		}
	};

	typedef std::deque<Layout> ContentLayout;
	ContentLayout layout;
	Point layoutPoint;

	struct {
		ieByte top;
		ieByte right;
		ieByte bottom;
		ieByte left;
	} margin;

public:
	ContentContainer(const Region& frame);
	virtual ~ContentContainer();

	// append a container to the end of the container. The container takes ownership of the span.
	virtual void AppendContent(Content* content);
	// Insert a span to a new position in the list. The container takes ownership of the span.
	virtual void InsertContentAfter(Content* newContent, const Content* existing);
	// removes the span from the container and transfers ownership to the caller.
	// Returns a non-const pointer to the removed span.
	Content* RemoveContent(const Content* content);
	virtual void DeleteContentsInRect(Region);

	Content* ContentAtPoint(const Point& p) const;
	const ContentList& Contents() { return contents; }

	const Region* ContentRegionForRect(const Region& rect) const;
	Region BoundingBoxForContent(const Content*) const;
	
	void SetMargin(ieByte top, ieByte right, ieByte bottom, ieByte left);
	inline void SetMargin(ieByte top, ieByte right, ieByte bottom) { SetMargin(top, right, bottom, right); }
	inline void SetMargin(ieByte top, ieByte right) { SetMargin(top, right, top, right); }
	inline void SetMargin(ieByte top) { SetMargin(top, top, top, top); }

protected:
	void SubviewAdded(View* view, View* parent);
	void LayoutContentsFrom(ContentList::const_iterator);
	void LayoutContentsFrom(const Content*);
	Content* RemoveContent(const Content* content, bool doLayout);
	const Layout& LayoutForContent(const Content*) const;

private:
	void SizeChanged(const Size& oldSize);

	void DrawSelf(Region drawFrame, const Region& clip);
};

// TextContainers can hold any content, but they represent a string of text that is divided into TextSpans
class TextContainer : public ContentContainer {
private:
	// default font/palette for adding plain text
	Font* font;
	Holder<Palette> palette;

public:
	TextContainer(const Region& frame, Font* font, Holder<Palette>);

	void AppendText(const String& text);
	void AppendText(const String& text, Font* fnt, Holder<Palette> pal);
	String Text() const;

	void SetPalette(Holder<Palette> pal);
	Holder<Palette> TextPalette() const { return palette; }
	const Font* TextFont() const { return font; }
};

}
#endif
