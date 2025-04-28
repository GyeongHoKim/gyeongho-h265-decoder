#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Callback function type for receiving YUV420P frame data.
 *
 * @param yuvBuffer Buffer containing the YUV420P frame data (contiguous memory)
 * @param bufferSize Total size of the buffer in bytes
 * @param width Frame width in pixels
 * @param height Frame height in pixels
 */
typedef void (*Yuv420pBufferCallback)(uint8_t* yuvBuffer, int bufferSize,
                                      int width, int height);

/**
 * @brief Enum representing the result of decoder functions.
 *
 * DecoderResult_Ok: Success
 * DecoderResult_FFmpegError: FFmpeg internal error
 */
typedef enum DecoderResult {
  DecoderResult_Ok = 0,         /**< Operation succeeded */
  DecoderResult_FFmpegError = 1 /**< FFmpeg-related error occurred */
} DecoderResult;

/**
 * @brief Callback function pointer for receiving YUV420P frame data.
 */
Yuv420pBufferCallback yuv420pBufferCallback = NULL;

/**
 * @brief FFmpeg H.265 decoder object.
 */
const AVCodec* decoder;

/**
 * @brief FFmpeg parser context (for parsing AnnexB streams).
 */
AVCodecParserContext* parserContext;

/**
 * @brief FFmpeg decoder context.
 */
AVCodecContext* decoderContext = NULL;

/**
 * @brief Packet object used for decoding.
 */
AVPacket* packet;

/**
 * @brief Object for holding decoded frames.
 */
AVFrame* frame;

/**
 * @brief Initializes the decoder and registers the callback. (WASM export)
 *
 * @param callback Callback function to receive decoded YUV420P frames
 * @return DecoderResult_Ok (success) or DecoderResult_FFmpegError (failure)
 */
DecoderResult init_decoder(Yuv420pBufferCallback callback) {
  DecoderResult ret = DecoderResult_Ok;

  do {
    decoder = avcodec_find_decoder(AV_CODEC_ID_H265);
    if (!decoder) {
      ret = DecoderResult_FFmpegError;
      break;
    }

    parserContext = av_parser_init(decoder->id);
    if (!parserContext) {
      ret = DecoderResult_FFmpegError;
      break;
    }

    decoderContext = avcodec_alloc_context3(decoder);
    if (!decoderContext) {
      ret = DecoderResult_FFmpegError;
      break;
    }

    if (avcodec_open2(decoderContext, decoder, NULL) < 0) {
      ret = DecoderResult_FFmpegError;
      break;
    }

    frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    if (!frame) {
      ret = DecoderResult_FFmpegError;
      break;
    }

    packet = av_packet_alloc();
    if (!packet) {
      ret = DecoderResult_FFmpegError;
      break;
    }

    yuv420pBufferCallback = callback;
  } while (0);

  return ret;
}

/**
 * @brief Converts a decoded AVFrame to a YUV420P buffer and passes it to the
 * callback.
 *
 * @param frame Pointer to the AVFrame to convert
 */
void sendFrameToCallback(AVFrame* frame) {
  int frameSize =
      av_image_get_buffer_size(frame->format, frame->width, frame->height, 1);
  if (frameSize < 0) {
    return;
  }

  uint8_t* yuvBuffer = (uint8_t*)av_malloc(frameSize);
  if (!yuvBuffer) {
    return;
  }

  int ret = av_image_copy_to_buffer(
      yuvBuffer, frameSize, (const uint8_t**)frame->data, frame->linesize,
      frame->format, frame->width, frame->height, 1);
  if (ret < 0) {
    av_free(yuvBuffer);
    return;
  }

  yuv420pBufferCallback(yuvBuffer, frameSize, frame->width, frame->height);
  av_free(yuvBuffer);
}

/**
 * @brief Decodes a single AVPacket and passes the frame to the callback.
 *
 * @param decCtx Decoder context
 * @param frame Frame to store the decoding result
 * @param packet Packet to decode (NULL for flush)
 * @return DecoderResult_Ok (success) or DecoderResult_FFmpegError (failure)
 */
static DecoderResult decodePacket(AVCodecContext* decCtx, AVFrame* frame,
                                  AVPacket* packet) {
  DecoderResult res = DecoderResult_Ok;
  int ret;

  ret = avcodec_send_packet(decCtx, packet);

  if (ret < 0) {
    res = DecoderResult_FFmpegError;
  } else {
    while (ret >= 0) {
      ret = avcodec_receive_frame(decCtx, frame);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        break;
      } else if (ret < 0) {
        res = DecoderResult_FFmpegError;
        break;
      }
      sendFrameToCallback(frame);
    }
  }

  return res;
}

/**
 * @brief Decodes a H.265 AnnexB format buffer. (WASM export)
 *
 * @param buffer Input buffer in H.265 AnnexB format
 * @param bufferSize Size of the input buffer in bytes
 * @return DecoderResult_Ok (success) or DecoderResult_FFmpegError (failure)
 */
DecoderResult decode_AnnexB_buffer(const uint8_t* buffer, size_t bufferSize) {
  DecoderResult ret = DecoderResult_Ok;

  while (bufferSize > 0) {
    int size = av_parser_parse2(parserContext, decoderContext, &packet->data,
                                &packet->size, buffer, bufferSize,
                                AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

    if (size < 0) {
      ret = DecoderResult_FFmpegError;
      break;
    }

    buffer += size;
    bufferSize -= size;

    if (packet->size) {
      ret = decodePacket(decoderContext, frame, packet);
      if (ret != DecoderResult_Ok) {
        break;
      }
    }
  }

  return ret;
}

/**
 * @brief Flushes any remaining data in the decoder. (WASM export)
 *
 * @return DecoderResult_Ok (success) or DecoderResult_FFmpegError (failure)
 */
DecoderResult flush_decoder() {
  return decodePacket(decoderContext, frame, NULL);
}

/**
 * @brief Releases all resources used by the decoder. (WASM export)
 *
 * @return DecoderResult_Ok (success)
 */
DecoderResult close_decoder() {
  DecoderResult ret = DecoderResult_Ok;

  if (parserContext != NULL) {
    av_parser_close(parserContext);
  }
  if (decoderContext != NULL) {
    avcodec_free_context(&decoderContext);
  }
  if (frame != NULL) {
    av_frame_free(&frame);
  }
  if (packet != NULL) {
    av_packet_free(&packet);
  }

  return ret;
}

/**
 * @brief (For testing) Main function. Not used in actual WASM usage. (WASM
 * export)
 */
int main(int argc, char** argv) { return 0; }