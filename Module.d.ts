declare interface Module {
  // Memory related
  _malloc(size: number): number;
  _free(pointer: number): void;
  HEAPU8: Uint8Array;

  // Decoder related
  _init_decoder(callback: number): void;
  _decode_AnnexB_buffer(buffer: number, size: number): void;

  // Function management
  addFunction(
    func: (...args: any[]) => void,
    signature: string
  ): number;
}

declare global {
  interface Window {
    Module: Module;
  }
  interface WorkerGlobalScope {
    Module: Module;
  }
  var Module: Module;
}

export type { Module };
