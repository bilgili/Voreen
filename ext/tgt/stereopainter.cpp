/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/stereopainter.h"
#include "tgt/camera.h"

namespace tgt {

StereoPainter::StereoPainter(GLCanvas* canvas, StereoMode stereoMode)
	: Painter(canvas)
{
	stereoMode_ = stereoMode;
	headTracker_ = NULL;
	trackingEnabled_ = false;
};

void StereoPainter::paint() {
	int width = getCanvas()->getSize().x;
	int height = getCanvas()->getSize().y;

	// Special handling for side-by-side stereo
	if (stereoMode_ == SIDE_BY_SIDE) {
		getCamera()->setRatio(float(width / 2) / height);
	}

	// Add headtracking offset
	if (trackingEnabled_ && headTracker_) {
		headTracker_->adaptCameraToHead();
	}

    switch (stereoMode_) {
		default: 
            // Falls through..
		case MONOSCOPIC:
			{
				glDrawBuffer(GL_BACK);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Render middle view
				// NOTE: Special headtracking frustums might require the stereo handling process
				// of the camera look() method so we set the eye separation to 0 and render
				// the left eye view instead of just:
				// getCamera()->look(Camera::EYE_MIDDLE);
				float initialEyeSep = getCamera()->getEyeSeparation();
				getCamera()->setEyeSeparation(0);
				getCamera()->look(Camera::EYE_LEFT);
				getCamera()->setEyeSeparation(initialEyeSep);
				render();
			}
			break;

		case FRAME_SEQUENTIAL:

            glDrawBuffer(GL_BACK);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render left eye view
			getCamera()->look(Camera::EYE_LEFT);
			render();

			getCanvas()->swap();

			// Render right eye view
			getCamera()->look(Camera::EYE_RIGHT);
			render();

			// For continuous rendering refresh canvas
			getCanvas()->update();
			break;

		case SIDE_BY_SIDE:
  
            glDrawBuffer(GL_BACK);   
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render left eye view
			glViewport(0, 0, width / 2, height);
			getCamera()->look(Camera::EYE_LEFT);
			render();

			// Render right eye view
			glViewport(width / 2, 0, width / 2, height);
			getCamera()->look(Camera::EYE_RIGHT);
			render();

		    glViewport(0, 0, width, height);
			break;

		case ANAGLYPH:

            glDrawBuffer(GL_BACK);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render left eye view
			glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
			getCamera()->look(Camera::EYE_LEFT);
			render();

			glClear(GL_DEPTH_BUFFER_BIT);

			// Render right eye view
			glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
			getCamera()->look(Camera::EYE_RIGHT);
			render();


			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			break;

		case QUADRO_PAGE_FLIPPING:
			// Render left eye view
    		glDrawBuffer(GL_BACK_LEFT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			getCamera()->look(Camera::EYE_LEFT);
			render();

			// Render right eye view
			glDrawBuffer(GL_BACK_RIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			getCamera()->look(Camera::EYE_RIGHT);
			render();
	
			break;

	   case VERTICAL_INTERLACED: 
			glDisable(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);

			glEnable(GL_STENCIL_TEST);
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);
			glStencilMask(0xFFFFFFFF);
			glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
				glLoadIdentity();
				glOrtho(0, width, 0, height, -1, 1);

				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
					glBegin(GL_LINES);
					for(int x = 0; x < width; x += 2) {
						glVertex2i(x, 0);
						glVertex2i(x, height);
					}
					glEnd();
				glPopMatrix();

				glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);

			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render right eye view
			getCamera()->look(Camera::EYE_RIGHT);
			glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
			render();
			
			// Render left eye view
			getCamera()->look(Camera::EYE_LEFT);
			glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);
			render();

			glDisable(GL_STENCIL_TEST);
			break;
	}

	// Reset camera
	if (trackingEnabled_ && headTracker_) {
		headTracker_->resetCamera();
	}

  	// Special handling for side-by-side stereo
	if (stereoMode_ == SIDE_BY_SIDE) {
		getCamera()->setRatio((float)width / height);
	}
}

} // namespace tgt
