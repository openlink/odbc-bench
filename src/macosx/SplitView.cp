/*
    File:		SplitView.cp
    
    Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "SplitView.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
#define kTViewClassID CFSTR( "com.apple.sample.SplitView" )

const ControlPartCode	kSubViewA			= 1;
const ControlPartCode	kSubViewSplitter	= 2;
const ControlPartCode	kSubViewB			= 3;

enum
{
	kImageHSplitter							= 0,
	kImageVSplitter,
	kImageHRidges,
	kImageVRidges,
	kImageHSplitterDisabled,
	kImageVSplitterDisabled,
	kImageHRidgesDisabled,
	kImageVRidgesDisabled,

	kImageCount
};

// -----------------------------------------------------------------------------
//	statics
// -----------------------------------------------------------------------------
//
CGImageRef*	SplitView::fImages = NULL;
UInt32		SplitView::fImageClientRefCount = 0;

// -----------------------------------------------------------------------------
//	SplitView constructor
// -----------------------------------------------------------------------------
//
SplitView::SplitView(
	HIViewRef			inControl )
	:	TView( inControl ),
		fSplitRatio( 0.5 ),
		fIsVertical( false ),
		fSubViewA( NULL ),
		fSubViewB( NULL )
{
	ChangeAutoInvalidateFlags( kAutoInvalidateOnActivate
			| kAutoInvalidateOnEnable
			| kAutoInvalidateOnHilite, 0 );
}

// -----------------------------------------------------------------------------
//	SplitView destructor
// -----------------------------------------------------------------------------
//
SplitView::~SplitView()
{
	int				i;
	
	// Without this instance, there is one less image client
	fImageClientRefCount--;
	
	// If there are no image clients, the images can be released
	if ( fImageClientRefCount == 0 && fImages != NULL )
	{
		for ( i = 0; i < kImageCount; i++ )
			CGImageRelease( fImages[ i ] );
		
		delete fImages;

		// Reset the static fImages ptr so it can be reinitialized if neccessary
		fImages = NULL;
	}
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind SplitView::GetKind()
{
	const ControlKind kMyKind = { 'TtsV', 'TtsV' };
	
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	GetBehaviors
//-----------------------------------------------------------------------------------
//
UInt32 SplitView::GetBehaviors()
{
	return TView::GetBehaviors() |  kControlSupportsEmbedding;
}

//-----------------------------------------------------------------------------------
//	Create
//-----------------------------------------------------------------------------------
//
OSStatus SplitView::Create(
	HIViewRef*			outControl,
	const HIRect*		inBounds,
	WindowRef			inWindow )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	ControlRef			root;
	
	// Register this class
	RegisterClass();

	// Make a new instantiation of this class
	err = HIObjectCreate( kTViewClassID, event, (HIObjectRef*) outControl );
	
	ReleaseEvent( event );

	if ( err == noErr )
	{
		if ( inWindow != NULL )
		{
			GetRootControl( inWindow, &root );
			HIViewAddSubview( root, *outControl );
		}

		HIViewSetFrame( *outControl, inBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	RegisterClass
//-----------------------------------------------------------------------------------
//	Register this class with the HIObject registry.
//
//	This API can be called multiple times, but will only register once.
//
void SplitView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kTViewClassID, Construct );
		sRegistered = true;
	}
}

//-----------------------------------------------------------------------------------
//	Construct
//-----------------------------------------------------------------------------------
//
OSStatus SplitView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new SplitView( inControl );
	
	return noErr;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	setting up the images.
//
OSStatus SplitView::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus			err;
	
	// Allow the parent's intialization to occur
	err = TView::Initialize( inEvent );
	require_noerr( err, CantInitializeParent );
	
	// Load the images if they aren't already
	if ( fImages == NULL )
	{
		CFStringRef			imageNames[ kImageCount ] = { 
								CFSTR( "HSplitter.png" ), CFSTR( "VSplitter.png" ),
								CFSTR( "HRidges.png" ), CFSTR( "VRidges.png" ),
								CFSTR( "HSplitterDisabled.png" ), CFSTR( "VSplitterDisabled.png" ),
								CFSTR( "HRidgesDisabled.png" ), CFSTR( "VRidgesDisabled.png" ) };
		CFURLRef			url;
		CGDataProviderRef	provider;
		int					i;

		fImages = new CGImageRef[ kImageCount ];
		require_action( fImages != NULL, CantMakeImageArray, err = memFullErr );
		
		// Load up the art work
		for ( i = 0; i < kImageCount; i++ )
		{
			url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), imageNames[ i ], NULL, NULL );
			require_action( url != NULL, CantGetURL, err = paramErr );
			
			provider = CGDataProviderCreateWithURL( url );
			
			fImages[ i ] = CGImageCreateWithPNGDataProvider( provider, NULL, false, kCGRenderingIntentDefault );
			require_action( fImages[ i ] != NULL, CantMakeImage, err = memFullErr );
			
			CGDataProviderRelease( provider );
		
			CFRelease( url );
		}
	}
	
	// Add 1 to the client count for these images
	fImageClientRefCount++;
	
CantMakeImage:
CantMakeImageArray:
CantGetURL:
CantInitializeParent:
	return err;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	Here's the fun stuff.  Just draw the splitter part of the view.
//
void SplitView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
#pragma unused( inLimitRgn )
	TRect				splitRect;
	TRect				ridgeRect;
	ControlPartCode		hilite;
	CGImageRef			splitter;
	CGImageRef			ridges;
	Boolean				drawEnabled = IsEnabled() && IsActive();

	CalculateRects( NULL, NULL, splitRect );

	hilite = GetHilite();
	
	if ( fIsVertical )
	{
		splitter = fImages[ drawEnabled ? kImageVSplitter : kImageVSplitterDisabled ];
		ridges = fImages[ drawEnabled ? kImageVRidges : kImageVRidgesDisabled ];
	}
	else
	{
		splitter = fImages[ drawEnabled ? kImageHSplitter : kImageHSplitterDisabled ];
		ridges = fImages[ drawEnabled ? kImageHRidges : kImageHRidgesDisabled ];
	}

	HIViewDrawCGImage( inContext, splitRect, splitter );
	ridgeRect.SetAroundCenter( splitRect.CenterX(), splitRect.CenterY(),
			CGImageGetWidth( ridges ), CGImageGetHeight( ridges ) );
	HIViewDrawCGImage( inContext, ridgeRect, ridges );
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//	Check to see if a point hits the view
//
ControlPartCode SplitView::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part;

	TRect				aRect;
	TRect				bRect;
	TRect				splitterRect;
	
	CalculateRects( aRect, bRect, splitterRect );

	// Is the mouse in the view?
	if ( aRect.Contains( inWhere ) )
		part = kSubViewA;
	else if ( splitterRect.Contains( inWhere ) )
		part = kSubViewSplitter;
	else if ( bRect.Contains( inWhere ) )
		part = kSubViewB;
	else
		part = kControlNoPart;

	return part;
}

//-----------------------------------------------------------------------------------
//	BoundsChanged
//-----------------------------------------------------------------------------------
//	Handler for bounds changed events
//
OSStatus SplitView::BoundsChanged(
	UInt32 				inOptions,
	const HIRect& 		inOriginalBounds,
	const HIRect& 		inCurrentBounds,
	RgnHandle 			inInvalRgn )
{
#pragma unused( inOptions, inOriginalBounds, inCurrentBounds, inInvalRgn )
	// Due to the change, the subviews need to resize
	SetSubViewBounds();
	
	// Due to the change, the view needs to redraw
	return Invalidate();
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus SplitView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus			err = noErr;
	TRect				bounds( Bounds() );
	Rect				qdBounds;
	
	if ( inPart == kControlContentMetaPart
			|| inPart == kControlStructureMetaPart
			/* || inPart == kControlOpaqueRegionMetaPart */ )
	{
		qdBounds = bounds;
		RectRgn( outRgn, &qdBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	CalculateRects
//-----------------------------------------------------------------------------------
//
void SplitView::CalculateRects(
	HIRect*			outRectA,
	HIRect*			outRectB,
	HIRect*			outSplitterRect )
{
	TRect			bounds( Bounds() );
	TRect			aRect, bRect, splitRect;

	// Use the ratio to calculate the size of the primary pane
	aRect = bounds;
	if ( fIsVertical )
		aRect.SetWidth( (int) (( aRect.Width() - GetVSplitterWidth() ) * fSplitRatio ) );
	else
		aRect.SetHeight( (int) (( aRect.Height() - GetHSplitterHeight() ) * fSplitRatio ) );

	// The splitter is always the same size
	splitRect = aRect;
	if ( fIsVertical )
	{
		splitRect.MoveBy( aRect.Width(), 0 );
		splitRect.SetWidth( GetVSplitterWidth() );
	}
	else
	{
		splitRect.MoveBy( 0, aRect.Height() );
		splitRect.SetHeight( GetHSplitterHeight() );
	}

	// The secondary pane just uses the remaining space of the parent view
	bRect = splitRect;
	if ( fIsVertical )
	{
		bRect.MoveBy( GetVSplitterWidth(), 0 );
		bRect.SetWidth( bounds.Width() - GetVSplitterWidth() - aRect.Width() );
	}
	else
	{
		bRect.MoveBy( 0, GetHSplitterHeight() );
		bRect.SetHeight( bounds.Height() - GetHSplitterHeight() - aRect.Height() );
	}
	
	// Send desired the rectangles back
	if ( outRectA )
		*outRectA = aRect;
	if ( outRectB )
		*outRectB = bRect;
	if ( outSplitterRect )
		*outSplitterRect = splitRect;
}

//-----------------------------------------------------------------------------------
//	GetData
//-----------------------------------------------------------------------------------
//
OSStatus SplitView::GetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	Size*				outSize,
	void*				inPtr )
{
	OSStatus			err = noErr;
	
	switch( inTag )
	{
		case kControlSplitViewOrientationTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( Boolean ) )
					err = errDataSizeMismatch;
				else
					( *(Boolean *) inPtr ) = fIsVertical;
			}
			*outSize = sizeof( Boolean );
			break;

		case kControlSplitViewSplitRatioTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( float ) )
					err = errDataSizeMismatch;
				else
					( *(float *) inPtr ) = fSplitRatio;
			}
			*outSize = sizeof( float );
			break;
		
		case kControlSplitViewSubViewA:
			if ( inPtr )
			{
				if ( inSize != sizeof( HIViewRef ) )
					err = errDataSizeMismatch;
				else
					( *(HIViewRef *) inPtr ) = fSubViewA;
			}
			*outSize = sizeof( HIViewRef );
			break;

		case kControlSplitViewSubViewB:
			if ( inPtr )
			{
				if ( inSize != sizeof( HIViewRef ) )
					err = errDataSizeMismatch;
				else
					( *(HIViewRef *) inPtr ) = fSubViewB;
			}
			*outSize = sizeof( HIViewRef );
			break;

		default:
			err = TView::GetData( inTag, inPart, inSize, outSize, inPtr );
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	SetData
//-----------------------------------------------------------------------------------
//
OSStatus SplitView::SetData(
	OSType				inTag,
	ControlPartCode		inPart,
	Size				inSize,
	const void*			inPtr )
{
	OSStatus			err = noErr;
	
	switch( inTag )
	{
		case kControlSplitViewOrientationTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( Boolean ) )
					err = errDataSizeMismatch;
				else
				{
					fIsVertical = ( *(Boolean *) inPtr );
					SetSubViewBounds();	// redraws
				}
			}
			break;

		case kControlSplitViewSplitRatioTag:
			if ( inPtr )
			{
				if ( inSize != sizeof( float ) )
					err = errDataSizeMismatch;
				else
				{
					fSplitRatio = ( *(float *) inPtr );
					SetSubViewBounds();	// redraws
				}
			}
			break;
		
		case kControlSplitViewSubViewA:
			if ( inPtr )
			{
				if ( inSize != sizeof( HIViewRef ) )
					err = errDataSizeMismatch;
				else
				{
					fSubViewA = ( *(HIViewRef *) inPtr );
					err = HIViewAddSubview( GetViewRef(), fSubViewA );
					SetSubViewBounds();	// redraws
				}
			}
			break;

		case kControlSplitViewSubViewB:
			if ( inPtr )
			{
				if ( inSize != sizeof( HIViewRef ) )
					err = errDataSizeMismatch;
				else
				{
					fSubViewB = ( *(HIViewRef *) inPtr );
					err = HIViewAddSubview( GetViewRef(), fSubViewB );
					SetSubViewBounds();	// redraws
				}
			}
			break;

		default:
			err = TView::SetData( inTag, inPart, inSize, inPtr );
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//
OSStatus SplitView::Track(
	TCarbonEvent&			inEvent,
	ControlPartCode*		outPart )
{
	OSStatus				err;
	HIPoint					where;
	HIPoint					lastWhere;
	TRect					hiBounds;
	ControlPartCode			part;
	ControlPartCode			lastPart = kControlNoPart;
	Point					qdPt;
	MouseTrackingResult		mouseResult;
	PixMapHandle			portPixMap;
	float					size;
	float					lastRatio;
	TRect					splitRect;
	float					clickOffsetMin;
	float					clickOffsetMax;

	// Extract the mouse location
	err = GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
			NULL, sizeof( HIPoint ), NULL, &where );
	require_noerr( err, ParameterMissing );

	// Need the port's pixMap's bounds to convert the mouse location
	portPixMap = GetPortPixMap( GetWindowPort( GetOwner() ) );

	// Check again to see if the mouse is in the view
	part = HitTest( where );
	
	// The tracking loop
	if ( part == kSubViewSplitter )
	{
		hiBounds = Bounds();
		
		// Set up the size
		if ( fIsVertical )
			size = hiBounds.Width() - GetVSplitterWidth();
		else
			size = hiBounds.Height() - GetHSplitterHeight();

		// Don't let the splitter get dragged offscreen.  Calculate
		// offsets that show where within the splitter the original
		// click occurred and track accordingly.
		CalculateRects( NULL, NULL, splitRect );
		if ( fIsVertical )
		{
			clickOffsetMin = where.x - splitRect.MinX();
			clickOffsetMax = splitRect.Width() - clickOffsetMin;
		}
		else
		{
			clickOffsetMin = where.y - splitRect.MinY();
			clickOffsetMax = splitRect.Height() - clickOffsetMin;
		}
		
		lastWhere = where;
		
		while ( true )
		{
			// If that changed, update
			if ( part != lastPart )
			{
				Hilite( part );
				lastPart = part;
			}
	
			// Watch the mouse for change
			err = TrackMouseLocation( (GrafPtr)-1L, &qdPt, &mouseResult );
	
			// Need to convert from global
			QDGlobalToLocalPoint( GetWindowPort( GetOwner() ), &qdPt );
			where.x = qdPt.h - (**portPixMap).bounds.left;
			where.y = qdPt.v - (**portPixMap).bounds.top;
			HIViewConvertPoint( &where, NULL, GetViewRef() );
	
			// Bail out when the mouse is released
			if ( mouseResult == kMouseTrackingMouseReleased )
				break;
	
			if ( fIsVertical )
			{
				if ( where.x < hiBounds.MinX() + clickOffsetMin )
					where.x = hiBounds.MinX() + clickOffsetMin;
				else if ( where.x > hiBounds.MaxX() - clickOffsetMax )
					where.x = hiBounds.MaxX() - clickOffsetMax;
			}
			else
			{
				if ( where.y < hiBounds.MinY() + clickOffsetMin )
					where.y = hiBounds.MinY() + clickOffsetMin;
				else if ( where.y > hiBounds.MaxY() - clickOffsetMax )
					where.y = hiBounds.MaxY() - clickOffsetMax;
			}
			
			// Keep track of the ratio
			lastRatio = fSplitRatio;

			// Update the ratio if the mouse moved
			if ( fIsVertical && lastWhere.x != where.x )
				fSplitRatio -= ( lastWhere.x - where.x ) / size;
			else if ( !fIsVertical && lastWhere.y != where.y )
				fSplitRatio -= ( lastWhere.y - where.y ) / size;

			// Update the subview sizes if the ratio changed (refreshes display)
			if ( lastRatio != fSplitRatio )
			{
				SetSubViewBounds();
				lastWhere = where;
			}

			// Check again to see if the mouse is in the view
			part = HitTest( where );
		}

		// Restore the original highlight
		Hilite( kControlNoPart );
	}
	
	// Send back the part upon which the mouse was released
	err = SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode,
			sizeof( ControlPartCode ), &lastPart ); 

	*outPart = lastPart;

ParameterMissing:
	return err;
}

//-----------------------------------------------------------------------------------
//	SetSubViewBounds
//-----------------------------------------------------------------------------------
//
OSStatus SplitView::SetSubViewBounds()
{
	HIRect				aRect;
	HIRect				bRect;
	
	CalculateRects( &aRect, &bRect, NULL );
	
	// Due to the change, the subviews need to resize
	if ( fSubViewA )
		verify_noerr( HIViewSetFrame( fSubViewA, &aRect ) );
	
	if ( fSubViewB )
		verify_noerr( HIViewSetFrame( fSubViewB, &bRect ) );

	// Due to the change, the view needs to redraw
	return Invalidate();
}

//-----------------------------------------------------------------------------------
//	GetVSplitterWidth
//-----------------------------------------------------------------------------------
//
float SplitView::GetVSplitterWidth()
{
	static float	vSplitterWidth = -1;
	
	if ( vSplitterWidth == -1 )
		vSplitterWidth = CGImageGetWidth( fImages[ kImageVSplitter ] );
	
	return vSplitterWidth;
}
	
//-----------------------------------------------------------------------------------
//	GetHSplitterHeight
//-----------------------------------------------------------------------------------
//
float SplitView::GetHSplitterHeight()
{
	static float	hSplitterHeight = -1;
	
	if ( hSplitterHeight == -1 )
		hSplitterHeight = CGImageGetHeight( fImages[ kImageHSplitter ] );
	
	return hSplitterHeight;
}
