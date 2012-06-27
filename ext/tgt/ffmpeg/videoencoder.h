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

#ifdef TGT_HAS_FFMPEG

#ifndef TGT_VIDEOENCODER_H
#define TGT_VIDEOENCODER_H

namespace tgt {

/**
 *
 * <emph>VideoEncoder</emph> is ready to receive frames by {@link #nextFrame(GLvoid*)}
 *  after call of {@link #startVideoEncoding(char*, unsigned int, unsigned int, unsigned int)}<br />
 * calling {@link #stopVideoEncoding()} will complete the encoding process (in any case)
 *
 * <h3>references:</h3>
 * <ul>
 * <li><a href="http://www.ffmpeg.com.cn/index.php/Output_example.c#write_video_frame">http://www.ffmpeg.com.cn/index.php/Output_example.c#write_video_frame</a></li>
 * </ul>
 */
class VideoEncoder {
public:
    VideoEncoder();
    ~VideoEncoder();

    /**
     * @param filePath target video file's full path (absolute or relative)
     * @param fps target video's fps
     * @param width source=target-video's width
     * @param height video's height
     */
    void startVideoEncoding(std::string filePath, const int fps,
                            const int width, const int height);// throw (NotMultipleOfTwoDimensionsException,CodecNotFoundOrNotLoadableException, NoWriteAccessException);
    /**
     * finish encoding
     */
    void stopVideoEncoding();

    /**
     * @param pixels passed frame must match <emph>width</emph>, <emph>height</emph> and <emph>pixelFormat</emph>, <emph>-Type</emph><br />
     * latest two assumed as GL_RGBA and GL_FLOAT
     */
    void nextFrame(GLvoid* pixels);//throw (TypeConversionFailed);

    static std::vector<std::string> getSupportedFormatsByFileEnding();

protected:
    /**
     * tgt logging category
     */
    static std::string loggerCat_;
};

} // namespace

#endif // TGT_VIDEOENCODER_H
#endif // TGT_HAS_FFMPEG
