/*
 *  gui.cpp
 *  wind
 *
 *  Created by damian on 13/07/11.
 *  Copyright 2011 frey damian@frey.co.nz. All rights reserved.
 *
 */


#include "Gui.h"
#include "ofMain.h"

static int BUTTON_GAP = 5;
static int BUTTON_WIDTH = 50;
static int BUTTON_HEIGHT = 20;

static int MAX_LEVEL_COLOR = 3;
static int LEVEL_COLORS[4] = { 0xFFC953, 0x96C0B1, 0xC4A093, 0x98B29C };

Gui* Gui::instance = NULL;

void Gui::setup( int w, int h )
{
	height = h;
	width = w;
}

GuiButton* Gui::addButton( string title, string tag, int x, int y, int depth )
{
	if ( getButton( tag ) == NULL )
	{
		GuiButton* b = new GuiButton( title, tag, x, y, depth );
		buttons.push_back( b );
		return b;
	}
	else
	{
		ofLog( OF_LOG_ERROR, "Gui::addButton: duplicate tag '%s'", tag.c_str() );
		return NULL;
	}
}

GuiButton* Gui::addButton( string title, string tag )
{
	GuiButton* b = addButton( title, tag, BUTTON_GAP + (BUTTON_WIDTH+BUTTON_GAP)*num_root_buttons++, height-BUTTON_HEIGHT-BUTTON_GAP, 0 );
	b->setVisible( true );
	root_buttons.push_back( b );
	return b;
}

GuiButton* Gui::addButton( string parent_tag, string title, string tag )
{
	GuiButton* parent = getButton( parent_tag );
	if ( !parent )
	{
		ofLog( OF_LOG_ERROR, "Gui::addButton: can't find parent tag '%s'", parent_tag.c_str() );
		return NULL;
	}
	GuiButton* b = addButton( title, tag, parent->getNextXPos(), parent->getNextYPos(), parent->getDepth()+1 );
	if ( b )
	{
		parent->addChild( b );
	}
}


// return the button with the given tag
GuiButton* Gui::getButton( string tag )
{
	for ( int i=0; i<buttons.size(); i++ )
	{
		if ( buttons[i]->getTag() == tag )
			return buttons[i];
	}
	return NULL;
}

void Gui::pointerDown( int x, int y )
{
	for ( int i=0; i<buttons.size(); i++ )
	{
		if ( buttons[i]->isVisible() && buttons[i]->checkHit( x, y ) )
		{
			// handle non-leaf (hierarchy)
			vector<GuiButton*> siblings = getSiblings( buttons[i] );
			if ( buttons[i]->isOpen() )
			{
				buttons[i]->setOpen( false );
				for ( int j=0; j<siblings.size(); j++ )
				{
					siblings[j]->flagDirty();
				}
			}
			else
			{
				// close siblings
				for ( int j=0; j<siblings.size(); j++ )
				{
					siblings[j]->setOpen( false );
				}
				if ( !buttons[i]->isLeaf() )
				{
					buttons[i]->setOpen( true );
				}
			}
			
			// tell listener
			listener->buttonPressCallback( buttons[i] );
			// must break, as buttonPressCallback might change buttons
			break;
		}
	}
}



void Gui::draw()
{
	ofSetHexColor( 0xffffff );
	ofNoFill();
	ofRect( 0, 0, width, height );
	// first draw non-visible dirty buttons (black squares)
	for ( int i=0; i<buttons.size(); i++ )
	{
		if ( buttons[i]->isDirty() )
			buttons[i]->draw( false );
	}
	// then draw visible dirty buttons (button content)
	for ( int i=0; i<buttons.size(); i++ )
	{
		if ( buttons[i]->isDirty() )
			buttons[i]->draw( true );
	}
}


// return a vector of siblings of this button
vector< GuiButton* > Gui::getSiblings( GuiButton* sibling )
{
	vector<GuiButton*> result;
	GuiButton* parent = getParent( sibling );
	if ( parent == NULL )
		return root_buttons;
	else
		return parent->getChildren();
}

// return the parent of this button
GuiButton* Gui::getParent( GuiButton* child )
{
	for ( int i=0; i<buttons.size(); i++ )
	{
		if ( !buttons[i]->isLeaf() && buttons[i]->isParentOf( child ) )
			return buttons[i];
	}
	return NULL;
}




GuiButton::GuiButton( string _title, string _tag, int _x, int _y, int _depth )
: title( _title ), tag( _tag ), x(_x), y(_y), depth(_depth), dirty( false ), open( false ), visible( false )
{	
}

bool GuiButton::checkHit( int hx, int hy )
{
	//ofLog( OF_LOG_VERBOSE, "%i %i [%i %i %i %i]", hx, hy, x, y, x+BUTTON_WIDTH, y+BUTTON_HEIGHT );
	return ( hx >= x && hy >= y && hx <= (x+BUTTON_WIDTH) && hy <=y+BUTTON_HEIGHT );		
}

void GuiButton::draw( bool drawVisible )
{
	if ( visible && drawVisible )
	{
		ofSetHexColor( LEVEL_COLORS[min(MAX_LEVEL_COLOR,depth)] );
		ofFill();
		ofRect( x, y, BUTTON_WIDTH, BUTTON_HEIGHT );	
		ofSetHexColor( 0x000000 );
		ofNoFill();
		ofRect( x, y, BUTTON_WIDTH, BUTTON_HEIGHT );
		ofDrawBitmapString( title, ofPoint( x+3, y+BUTTON_HEIGHT-3 ) );
		dirty = false;
	}
	else if ( !visible && !drawVisible )
	{
		ofSetColor( 0, 0, 0 );
		ofFill();
		ofRect( x, y, BUTTON_WIDTH, BUTTON_HEIGHT );	
		dirty = false;
	}		
}

void GuiButton::setOpen( bool tf )
{
	if ( open != tf )
	{
		open = tf;
		if ( open ) 
		{
			for ( int i=0; i<children.size(); i++ )
			{
				children[i]->setVisible( true );
			}
		}
		else
		{
			for ( int i=0; i<children.size(); i++ )
			{
				children[i]->setVisible( false );
				children[i]->setOpen( false );
			}
		}
	}
}

int GuiButton::getNextXPos()
{
	if ( (depth%2)==0 )
		return x;
	else
		return x + (children.size()+1)*BUTTON_WIDTH;
}

int GuiButton::getNextYPos()
{
	if ( (depth%2)==0 )
		return y - (children.size()+1)*BUTTON_HEIGHT;
	else
		return y;
}