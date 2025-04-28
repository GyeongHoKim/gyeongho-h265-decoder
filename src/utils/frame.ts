export const yuv420ToVideoFrame = (yuv420: Uint8Array, width: number, height: number, timestamp?: number): VideoFrame => {
  const ySize = width * height;
  const uSize = (width / 2) * (height / 2);

  const yOffset = 0;
  const uOffset = yOffset + ySize;
  const vOffset = uOffset + uSize;

  const layout: PlaneLayout[] = [
    {
      offset: yOffset,
      stride: width,
    },
    {
      offset: uOffset,
      stride: width / 2,
    },
    {
      offset: vOffset,
      stride: width / 2,
    },
  ];
  const init: VideoFrameBufferInit = {
    timestamp: timestamp ?? performance.now(),
    codedHeight: height,
    codedWidth: width,
    format: "I420",
    layout,
  };

  return new VideoFrame(yuv420.buffer, init);
};