// The monitor. All three.js lives here: an 8-bit index texture and a 256-entry palette texture on one quad.
import * as THREE from 'three';

const VERTEX_SHADER = `
  varying vec2 vUv;
  void main() {
    vUv = uv;
    gl_Position = vec4(position.xy, 0.0, 1.0);
  }
`;

// Row 0 of the framebuffer is the top of the picture, so v is flipped.
const FRAGMENT_SHADER = `
  uniform sampler2D indexTexture;
  uniform sampler2D paletteTexture;
  varying vec2 vUv;
  void main() {
    float index = texture2D(indexTexture, vec2(vUv.x, 1.0 - vUv.y)).r * 255.0;
    gl_FragColor = vec4(texture2D(paletteTexture, vec2((index + 0.5) / 256.0, 0.5)).rgb, 1.0);
  }
`;

function nearest(texture) {
  texture.minFilter = THREE.NearestFilter;
  texture.magFilter = THREE.NearestFilter;
  texture.needsUpdate = true;
  return texture;
}

export class Screen {
  constructor(canvas) {
    this.renderer = new THREE.WebGLRenderer({ canvas, antialias: false });
    this.renderer.setPixelRatio(1);
    this.paletteBytes = new Uint8Array(256 * 4).fill(255);
    this.paletteTexture = nearest(new THREE.DataTexture(this.paletteBytes, 256, 1, THREE.RGBAFormat));
    this.indexTexture = null;
    this.material = new THREE.ShaderMaterial({
      uniforms: { indexTexture: { value: null }, paletteTexture: { value: this.paletteTexture } },
      vertexShader: VERTEX_SHADER,
      fragmentShader: FRAGMENT_SHADER,
      depthTest: false,
    });
    this.scene = new THREE.Scene();
    this.scene.add(new THREE.Mesh(new THREE.PlaneGeometry(2, 2), this.material));
    this.camera = new THREE.OrthographicCamera(-1, 1, 1, -1, 0, 1);
  }

  /** The demo changes resolution once (credits), which swaps the machine's front buffer. */
  useFrontBuffer(machine) {
    if (this.indexTexture) {
      this.indexTexture.dispose();
    }
    this.indexTexture = nearest(new THREE.DataTexture(machine.front, machine.width, machine.height, THREE.RedFormat));
    this.material.uniforms.indexTexture.value = this.indexTexture;
    this.renderer.setSize(machine.width, machine.height, false);
  }

  /** Show what the machine's monitor shows: its front buffer through its 6-bit DAC. */
  present(machine) {
    if (!this.indexTexture || this.indexTexture.image.data !== machine.front) {
      this.useFrontBuffer(machine);
    }
    const dac = machine.palette;
    const rgba = this.paletteBytes;
    for (let i = 0; i < 256; i++) {
      for (let component = 0; component < 3; component++) {
        const value = dac[i * 3 + component];
        rgba[i * 4 + component] = (value << 2) | (value >> 4);
      }
    }
    this.paletteTexture.needsUpdate = true;
    this.indexTexture.needsUpdate = true;
    this.renderer.render(this.scene, this.camera);
  }
}
