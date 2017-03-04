#include <fstream>
#include <iostream>
#include <string>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "eglrender.h"

using namespace std;

#define CHECK(a)  if (!(a)) throw runtime_error("error");

string const VertexShaderCode = R"gl(
//#version 200 es
attribute vec3 vertexPosition_modelspace;
attribute vec3 vertexColor;

varying vec3 fragmentColor;
uniform mat4 MVP;

void main(){

        gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

        fragmentColor = vertexColor;
}
)gl";

string const FragmentShaderCode = R"gl(
//#version 200 es
precision mediump float;
varying vec3 fragmentColor;
void main(){
        gl_FragColor = vec4(fragmentColor, 1);
}
)gl";

GLuint LoadShaders () {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file

	// Read the Fragment Shader code from the file
	GLint Result = GL_FALSE;
	int InfoLogLength;


    cout << "Compiling vertex shader..." << endl;
    char const *ptr = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &ptr , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0){
		string msg;
        msg.resize(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &msg[0]);
        cerr << msg << endl;
	}
    CHECK(Result);

    cout << "Compiling fragment shader..." << endl;

    ptr = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &ptr , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		string msg;
        msg.resize(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &msg[0]);
        cerr << msg << endl;
	}
    CHECK(Result);

	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		string msg;
        msg.resize(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &msg[0]);
        cerr << msg << endl;
	}
    CHECK(Result);
	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static const float MVP[16] = {
		-0.806666, -0.737824, -0.687368, -0.685994,
		0, 1.53713, -0.515526, -0.514496,
		-1.07555, 0.553368, 0.515526, 0.514496,
		0, 0, 5.64243, 5.83095
};

int main( int argc, char **argv )
{
    string input;
    string output;
    unsigned width;
    unsigned height;
    unsigned repeat;
    {
        namespace po = boost::program_options;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message.")
            ("input", po::value(&input), "")
            ("output", po::value(&output), "")
            ("width", po::value(&width)->default_value(800), "")
            ("height", po::value(&height)->default_value(600), "")
            ("repeat", po::value(&repeat)->default_value(1), "")
            ;

        po::positional_options_description p;
        p.add("input", 1);
        p.add("output", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                         options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help") || input.empty() || output.empty()) {
            cout << "Usage:" << endl;
            cout << "\t" << argv[0] << " [options] <input> <output>" << endl;
            cout << desc;
            cout << endl;
            return 0;
        }
    }

    vector<float> vertex_buffer_data;
    vector<float> color_buffer_data;

    {
        ifstream is(input.c_str());
		float p0, p1, p2, c0, c1, c2;
        while (is >> p0 >> p1 >> p2 >> c0 >> c1 >> c2) {
            vertex_buffer_data.emplace_back(p0);
            vertex_buffer_data.emplace_back(p1);
            vertex_buffer_data.emplace_back(p2);
            color_buffer_data.emplace_back(c0);
            color_buffer_data.emplace_back(c1);
            color_buffer_data.emplace_back(c2);
        }
    }

	// Initialise GLFW
    EglRenderCtx ctx(width, height);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 


	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders();

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    /*
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data[0]) * vertex_buffer_data.size(), 
                                  &vertex_buffer_data[0], GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data[0]) * color_buffer_data.size(),
                                  &color_buffer_data[0], GL_STATIC_DRAW);
                                  */
    auto VertexCoordID = glGetAttribLocation (programID, "vertexPosition_modelspace" );
    auto VertexColorID = glGetAttribLocation (programID, "vertexColor" );
    cout << VertexCoordID << " === " << VertexColorID << endl;

    cv::Mat image;
    for (unsigned i = 0; i < repeat; ++i) {


        glViewport(0, 0, width, height);
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

	    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, MVP);

		// 1rst attribute buffer : vertices
		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glEnableVertexAttribArray(VertexCoordID);
		glVertexAttribPointer(
			VertexCoordID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			&vertex_buffer_data[0]
		);
		glEnableVertexAttribArray(VertexColorID);
		glVertexAttribPointer(
			VertexColorID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
            &color_buffer_data[0]
		);

		glDrawArrays(GL_TRIANGLES, 0, vertex_buffer_data.size()/3);
		glDisableVertexAttribArray(VertexCoordID);
		glDisableVertexAttribArray(VertexColorID);

        image = ctx.render();
	}

    cv::imwrite(output, image);

	glDeleteProgram(programID);

	return 0;
}
