class LineBreakTransformer {
  chunks = "";

  transform(chunk, controller) {
    this.chunks += chunk;
    const lines = this.chunks.split("\n");
    this.chunks = lines.pop();
    lines.forEach((line) => controller.enqueue(line));
  }
}

class JsonTransformer {
  transform(chunk, controller) {
    try {
      controller.enqueue(JSON.parse(chunk));
    } catch (e) {
      console.warn("Could not parse chunk as JSON, discarding: ", chunk);
    }
  }
}

class SerialHandler {
  messageCallback = null;

  async connect() {
    const port = await navigator.serial.requestPort();
    await port.open({ baudRate: 115200 });  // Ensure baud rate matches the ESP32 settings

    const decoder = new TextDecoderStream();
    port.readable.pipeTo(decoder.writable);
    const inputStream = decoder.readable
      .pipeThrough(new TransformStream(new LineBreakTransformer()))
      .pipeThrough(new TransformStream(new JsonTransformer()));
    const reader = inputStream.getReader();

    while (true) {
      try {
        const { value, done } = await reader.read();
        if (done) {
          console.log('[readLoop] DONE', done);
          reader.releaseLock();
          break;
        }
        if (value && this.messageCallback) {
          this.messageCallback(value);
        }
      } catch (error) {
        console.error('Error reading data: ', error);
        break;
      }
    }
  }
}
