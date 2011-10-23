"use strict";

/**
* Initialises WebGL and creates the 3D scene.
*/
function loadScene()
{
	// Get the canvas element
	var canvas = document.getElementById("webGLCanvas");
	// Get the WebGL context
	var gl = canvas.getContext("experimental-webgl");
	// Check whether the WebGL context is available or not
	// if it's not available exit
	if(!gl)
	{
		alert("There's no WebGL context available.");
		return;
	}
	// Set the viewport to the canvas width and height
	gl.viewport(0, 0, canvas.width, canvas.height);

	// Load the vertex shader that's defined in a separate script
	// block at the top of this page.
	// More info about shaders: http://en.wikipedia.org/wiki/Shader_Model
	// More info about GLSL: http://en.wikipedia.org/wiki/GLSL
	// More info about vertex shaders: http://en.wikipedia.org/wiki/Vertex_shader

	// Grab the script element
	var vertexShaderScript = document.getElementById("shader-vs");
	// Create a vertex shader object
	var vertexShader = gl.createShader(gl.VERTEX_SHADER);
	// Load the shader with the source strings from the script element
	gl.shaderSource(vertexShader, vertexShaderScript.text);
	// Compile the shader source code string
	gl.compileShader(vertexShader);
	// Check if the shader has compiled without errors
	if(!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
		alert("Couldn't compile the vertex shader");
		// Clean up
		gl.deleteShader(vertexShader);
		return;
	}

	// Load the fragment shader that's defined in a separate script
	// block at the top of this page.
	// More info about fragment shaders: http://en.wikipedia.org/wiki/Fragment_shader
	var fragmentShaderScript = document.getElementById("shader-fs");
	// Create a fragment shader object
	var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	// Load the shader with the source strings from the script element
	gl.shaderSource(fragmentShader, fragmentShaderScript.text);
	// Compile the shader source code string
	gl.compileShader(fragmentShader);
	// Check if the shader has compiled without errors
	if(!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
		alert("Couldn't compile the fragment shader");
		// Clean up
		gl.deleteShader(fragmentShader);
		return;
	}

	// Create a shader program. From the OpenGL documentation:
	// A program object is an object to which shader objects can be attached.
	// This provides a mechanism to specify the shader objects that will be linked to
	// create a program. It also provides a means for checking the compatibility of the
	// shaders that will be used to create a program (for instance, checking the
	// compatibility between a vertex shader and a fragment shader).
	gl.program = gl.createProgram();
	// Attach the vertex shader to the program
	gl.attachShader(gl.program, vertexShader);
	// Attach the fragment shader to the program
	gl.attachShader(gl.program, fragmentShader);
	// Before we can use the shaders for rendering, we have to link the program
	// object.
	gl.linkProgram(gl.program);
	// Check the status of the link operation to see if it was linked without
	// errors.
	if (!gl.getProgramParameter(gl.program, gl.LINK_STATUS)) {
		alert("Unable to initialise shaders");
		// Clean up
		gl.deleteProgram(gl.program);
		gl.deleteProgram(vertexShader);
		gl.deleteProgram(fragmentShader);
		return;
	}
	// Install the program as part of the current rendering state
	gl.useProgram(gl.program);
	// Get the vertexPosition attribute from the linked shader program
	var vertexPosition = gl.getAttribLocation(gl.program, "vertexPosition");
	// Enable the vertexPosition vertex attribute array. If enabled, the array
	// will be accessed an used for rendering when calls are made to commands like
	// gl.drawArrays, gl.drawElements, etc.
	gl.enableVertexAttribArray(vertexPosition);

	// Clear the color buffer (r, g, b, a) with the specified color
	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	// Clear the depth buffer. The value specified is clamped to the range [0,1].
	// More info about depth buffers: http://en.wikipedia.org/wiki/Depth_buffer
	gl.clearDepth(1.0);
	// Enable depth testing. This is a technique used for hidden surface removal.
	// It assigns a value (z) to each pixel that represents the distance from this
	// pixel to the viewer. When another pixel is drawn at the same location the z
	// values are compared in order to determine which pixel should be drawn.
	gl.enable(gl.DEPTH_TEST);
	// Specify which function to use for depth buffer comparisons. It compares the
	// value of the incoming pixel against the one stored in the depth buffer.
	// Possible values are (from the OpenGL documentation):
	// GL_NEVER - Never passes.
	// GL_LESS - Passes if the incoming depth value is less than the stored depth value.
	// GL_EQUAL - Passes if the incoming depth value is equal to the stored depth value.
	// GL_LEQUAL - Passes if the incoming depth value is less than or equal to the stored depth value.
	// GL_GREATER - Passes if the incoming depth value is greater than the stored depth value.
	// GL_NOTEQUAL - Passes if the incoming depth value is not equal to the stored depth value.
	// GL_GEQUAL - Passes if the incoming depth value is greater than or equal to the stored depth value.
	// GL_ALWAYS - Always passes.
	gl.depthFunc(gl.LEQUAL);

	// Now create a shape.
	// First create a vertex buffer in which we can store our data.
	var vertexBuffer = gl.createBuffer();
	// Bind the buffer object to the ARRAY_BUFFER target.
	gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
	// Specify the vertex positions (x, y, z)
	var vertices = new Float32Array([
			0.0, 1.0, 4.0,
			-1.0, -1.0, 4.0,
			1.0, -1.0, 4.0
			]);

	// Creates a new data store for the vertices array which is bound to the ARRAY_BUFFER.
	// The third paramater indicates the usage pattern of the data store. Possible values are
	// (from the OpenGL documentation):
	// The frequency of access may be one of these:
	// STREAM - The data store contents will be modified once and used at most a few times.
	// STATIC - The data store contents will be modified once and used many times.
	// DYNAMIC - The data store contents will be modified repeatedly and used many times.
	// The nature of access may be one of these:
	// DRAW - The data store contents are modified by the application, and used as the source for
	// GL drawing and image specification commands.
	// READ - The data store contents are modified by reading data from the GL, and used to return
	// that data when queried by the application.
	// COPY - The data store contents are modified by reading data from the GL, and used as the source
	// for GL drawing and image specification commands.
	gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

	// Clear the color buffer and the depth buffer
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);


	// Define the viewing frustum parameters
	// More info: http://en.wikipedia.org/wiki/Viewing_frustum
	// More info: http://knol.google.com/k/view-frustum
	var fieldOfView = 30.0;
	var aspectRatio = canvas.width / canvas.height;
	var nearPlane = 1.0;
	var farPlane = 10000.0;
	var top = nearPlane * Math.tan(fieldOfView * Math.PI / 360.0);
	var bottom = -top;
	var right = top * aspectRatio;
	var left = -right;


	// Create the perspective matrix. The OpenGL function that's normally used for this,
	// glFrustum() is not included in the WebGL API. That's why we have to do it manually here.
	// More info: http://www.cs.utk.edu/~vose/c-stuff/opengl/glFrustum.html
	var a = (right + left) / (right - left);
	var b = (top + bottom) / (top - bottom);
	var c = (farPlane + nearPlane) / (farPlane - nearPlane);
	var d = (2 * farPlane * nearPlane) / (farPlane - nearPlane);
	var x = (2 * nearPlane) / (right - left);
	var y = (2 * nearPlane) / (top - bottom);
	var perspectiveMatrix = [
		x, 0, a, 0,
		0, y, b, 0,
		0, 0, c, d,
		0, 0, -1, 0
			];

	// Create the modelview matrix
	// More info about the modelview matrix: http://3dengine.org/Modelview_matrix
	// More info about the identity matrix: http://en.wikipedia.org/wiki/Identity_matrix
	var modelViewMatrix = [
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
			];
	// Get the vertex position attribute location from the shader program
	var vertexPosAttribLocation = gl.getAttribLocation(gl.program, "vertexPosition");
	// Specify the location and format of the vertex position attribute
	gl.vertexAttribPointer(vertexPosAttribLocation, 3.0, gl.FLOAT, false, 0, 0);
	// Get the location of the "modelViewMatrix" uniform variable from the
	// shader program
	var uModelViewMatrix = gl.getUniformLocation(gl.program, "modelViewMatrix");
	// Get the location of the "perspectiveMatrix" uniform variable from the
	// shader program
	var uPerspectiveMatrix = gl.getUniformLocation(gl.program, "perspectiveMatrix");
	// Set the values
	gl.uniformMatrix4fv(uModelViewMatrix, false, new Float32Array(perspectiveMatrix));
	gl.uniformMatrix4fv(uPerspectiveMatrix, false, new Float32Array(modelViewMatrix));
	// Draw the triangles in the vertex buffer. The first parameter specifies what
	// drawing mode to use. This can be POINTS, LINE_STRIP, LINE_LOOP, LINES, TRIANGLE_STRIP,
	// TRIANGLE_FAN and TRIANGLES
	gl.drawArrays(gl.TRIANGLES, 0, vertices.length / 3.0);
	gl.flush();
}

// Global vars
var canvascontainer, canvas, gl;
var vertex_shader, fragment_shader, shader_program;
var vertex_buffer;

function init_global_vars() {
	canvascontainer = document.getElementById('canvascontainer');
	canvas = document.getElementById('canvas');
	gl = canvas.getContext('experimental-webgl');
}

function resize_canvas() {
	// https://developer.mozilla.org/en/Determining_the_dimensions_of_elements
	var width = canvascontainer.clientWidth;
	var height = canvascontainer.clientHeight;

	console.log('Resizing to', width, height);

	// http://pl4n3.blogspot.com/2010/02/html5-canvas-with-variable-size.html
	canvas.width = width;
	canvas.height = height;
	gl.viewport(0, 0, width, height);
}

function init_shaders() {
	// Global vars: fragment_shader, shader_program
	var vs_text = document.getElementById('shader-vs').text;
	var fs_text = document.getElementById('shader-fs').text;

	// The vertex shader
	vertex_shader = gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertex_shader, vs_text);
	gl.compileShader(vertex_shader);

	if (!gl.getShaderParameter(vertex_shader, gl.COMPILE_STATUS)) {
		abort_with_message('Error while compiling the vertex shader');

		// .deleteShader() is optional here, because the shader object is
		// automatically deleted when WebGLShader object is destroyed.
		// http://www.khronos.org/registry/webgl/specs/1.0/#5.13.9
		//gl.deleteShader(fragment_shader);
		vertex_shader = null;

		return false;
	}

	// The fragment shader
	fragment_shader = gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragment_shader, fs_text);
	gl.compileShader(fragment_shader);

	if (!gl.getShaderParameter(fragment_shader, gl.COMPILE_STATUS)) {
		abort_with_message('Error while compiling the fragment shader');

		fragment_shader = null;

		return false;
	}

	// The shader program
	shader_program = gl.createProgram();
	gl.attachShader(shader_program, vertex_shader);
	gl.attachShader(shader_program, fragment_shader);
	gl.linkProgram(shader_program);

	if (!gl.getProgramParameter(shader_program, gl.LINK_STATUS)) {
		abort_with_message('Error while linking the shader program');

		shader_program = null;
		fragment_shader = null;

		return false;
	}

	return true;
}
function init_other_objects() {
	var vertices = new Float32Array([
		+1.0, -1.0,  // topright
		+1.0, +1.0,  // bottomright
		-1.0, -1.0,  // topleft
		-1.0, +1.0   // bottomleft
	]);
	vertex_buffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer);
	gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
}

function paint() {
	var pos = gl.getAttribLocation(shader_program, "pos");
	gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer);
	gl.vertexAttribPointer(pos, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(pos);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	gl.flush();
    gl.disableVertexAttribArray(pos);
}

function abort_with_message(text) {
	var nowebgl = document.getElementById('nowebgl');
	canvascontainer.style.display = 'none';
	nowebgl.style.display = 'block';
	nowebgl.innerHTML = text;
}

function init() {
	// Based on http://www.rozengain.com/blog/2010/02/22/beginning-webgl-step-by-step-tutorial/

	init_global_vars();
	if(!gl) {
		abort_with_message('WebGL not supported');
		return;
	}

	resize_canvas();
	window.addEventListener('resize', resize_canvas, false);

	var stat = init_shaders();
	if (!stat) {
		return;
	}
	init_other_objects();

	gl.useProgram(shader_program);

	gl.clearDepth(1.0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);

	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	setTimeout(paint, 1000);
}

window.addEventListener('load', init, false);
