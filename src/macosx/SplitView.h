/*
    File:		SplitView.h
    
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

#ifndef SplitView_H_
#define SplitView_H_

#include <Carbon/Carbon.h>

#include "TView.h"

class SplitView
	: public TView
{
public:
	static OSStatus			Create(
								HIViewRef*			outControl,
								const HIRect*		inBounds = NULL,
								WindowRef			inWindow = NULL );

	// Accessors

protected:
	// Contstructor/Destructor
							SplitView(
								HIViewRef			inControl );
	virtual					~SplitView();

	// Accessors
	virtual ControlKind		GetKind();
	virtual UInt32			GetBehaviors();

	// Handlers
	virtual OSStatus		BoundsChanged(
								UInt32				inOptions,
								const HIRect&		inOriginalBounds,
								const HIRect&		inCurrentBounds,
								RgnHandle			inInvalRgn );
	virtual void			Draw(
								RgnHandle			inLimitRgn,
								CGContextRef		inContext );
	virtual OSStatus		GetData(
								OSType				inTag,
								ControlPartCode		inPart,
								Size				inSize,
								Size*				outSize,
								void*				inPtr );
	virtual OSStatus		GetRegion(
								ControlPartCode		inPart,
								RgnHandle			outRgn );
	virtual ControlPartCode	HitTest(
								const HIPoint&		inWhere );
	virtual OSStatus		Initialize(
								TCarbonEvent&		inEvent );
	virtual OSStatus		SetData(
								OSType				inTag,
								ControlPartCode		inPart,
								Size				inSize,
								const void*			inPtr );
	virtual OSStatus		Track(
								TCarbonEvent&		inEvent,
								ControlPartCode*	outPartHit );

private:
	float					GetVSplitterWidth();
	float					GetHSplitterHeight();

	static void 			RegisterClass();
	static OSStatus			Construct(
								ControlRef			inControl,
								TView**				outView );
	void					CalculateRects(
								HIRect*				outRectA,
								HIRect*				outRectB,
								HIRect*				outSplitterRect );
	OSStatus				SetSubViewBounds();

	float					fSplitRatio;
	Boolean					fIsVertical;
	HIViewRef				fSubViewA;
	HIViewRef				fSubViewB;
	
	static CGImageRef*		fImages;
	static UInt32			fImageClientRefCount;
};

const UInt32	kControlSplitViewOrientationTag = 'svot';	// Boolean -- true == vertical
const UInt32	kControlSplitViewSplitRatioTag = 'svsr';	// float -- % of split view is subview A
const UInt32	kControlSplitViewSubViewA = 'sv_a';			// HIViewRef -- the view ref for subview A
const UInt32	kControlSplitViewSubViewB = 'sv_b';			// HIViewRef -- the view ref for subview B

#endif // SplitView_H_