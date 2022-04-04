//
// Simple .obj viewer(vertex only)
//
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <cmath>
#include <cassert>
#include <cstring>
#include <algorithm>

#if defined(ENABLE_ZLIB)
#include <zlib.h>
#endif

#if defined(ENABLE_ZSTD)
#include <zstd.h>
#endif

#include <GL/glew.h>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <GLFW/glfw3.h>

#include "trackball.h"

#define TINYOBJ_LOADER_OPT_IMPLEMENTATION
#include "tinyobj_loader_opt.h"

typedef struct {
  GLuint vb;    // vertex buffer
  int numTriangles;
} DrawObject;

std::vector<DrawObject> gDrawObjects;

int width = 768;
int height = 768;

double prevMouseX, prevMouseY;
bool mouseLeftPressed;
bool mouseMiddlePressed;
bool mouseRightPressed;
float curr_quat[4];
float prev_quat[4];
float eye[3], lookat[3], up[3];

GLFWwindow* window;

void CheckErrors(std::string desc) {
  GLenum e = glGetError();
  if (e != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error in \"%s\": %d (%d)\n", desc.c_str(), e, e);
    exit(20);
  }
}

void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
  float v10[3];
  v10[0] = v1[0] - v0[0];
  v10[1] = v1[1] - v0[1];
  v10[2] = v1[2] - v0[2];

  float v20[3];
  v20[0] = v2[0] - v0[0];
  v20[1] = v2[1] - v0[1];
  v20[2] = v2[2] - v0[2];

  N[0] = v20[1] * v10[2] - v20[2] * v10[1];
  N[1] = v20[2] * v10[0] - v20[0] * v10[2];
  N[2] = v20[0] * v10[1] - v20[1] * v10[0];

  float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
  if (len2 > 0.0f) {
    float len = sqrtf(len2);

    N[0] /= len;
    N[1] /= len;
  }
}

const char *mmap_file(size_t *len, const char* filename)
{
  (*len) = 0;
#ifdef _WIN32
  HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  assert(file != INVALID_HANDLE_VALUE);

  HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
  assert(fileMapping != INVALID_HANDLE_VALUE);

  LPVOID fileMapView = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
  auto fileMapViewChar = (const char*)fileMapView;
  assert(fileMapView != NULL);

  LARGE_INTEGER fileSize;
  fileSize.QuadPart = 0;
  GetFileSizeEx(file, &fileSize);

  (*len) = static_cast<size_t>(fileSize.QuadPart);
  return fileMapViewChar;

#else

  FILE* f = fopen(filename, "rb" );
  if (!f) {
    fprintf(stderr, "Failed to open file : %s\n", filename);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  long fileSize = ftell(f);
  fclose(f);

  if (fileSize < 16) {
    fprintf(stderr, "Empty or invalid .obj : %s\n", filename);
    return nullptr;
  }

  struct stat sb;
  char *p;
  int fd;

  fd = open (filename, O_RDONLY);
  if (fd == -1) {
    perror ("open");
    return nullptr;
  }

  if (fstat (fd, &sb) == -1) {
    perror ("fstat");
    return nullptr;
  }

  if (!S_ISREG (sb.st_mode)) {
    fprintf (stderr, "%s is not a file\n", filename);
    return nullptr;
  }

  p = (char*)mmap (0, fileSize, PROT_READ, MAP_SHARED, fd, 0);

  if (p == MAP_FAILED) {
    perror ("mmap");
    return nullptr;
  }

  if (close (fd) == -1) {
    perror ("close");
    return nullptr;
  }

  (*len) = fileSize;

  return p;

#endif
}

bool gz_load(std::vector<char>* buf, const char* filename)
{
#ifdef ENABLE_ZLIB
    gzFile file;
    file = gzopen (filename, "r");
    if (! file) {
        fprintf (stderr, "gzopen of '%s' failed: %s.\n", filename,
                 strerror (errno));
        exit (EXIT_FAILURE);
        return false;
    }
    while (1) {
        int err;
        int bytes_read;
        unsigned char buffer[1024];
        bytes_read = gzread (file, buffer, 1024);
        buf->insert(buf->end(), buffer, buffer + 1024);
        //printf ("%s", buffer);
        if (bytes_read < 1024) {
            if (gzeof (file)) {
                break;
            }
            else {
                const char * error_string;
                error_string = gzerror (file, & err);
                if (err) {
                    fprintf (stderr, "Error: %s.\n", error_string);
                    exit (EXIT_FAILURE);
                    return false;
                }
            }
        }
    }
    gzclose (file);
    return true;
#else
  return false;
#endif
}

#ifdef ENABLE_ZSTD
static off_t fsize_X(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0) return st.st_size;
    /* error */
    printf("stat: %s : %s \n", filename, strerror(errno));
    exit(1);
}

static FILE* fopen_X(const char *filename, const char *instruction)
{
    FILE* const inFile = fopen(filename, instruction);
    if (inFile) return inFile;
    /* error */
    printf("fopen: %s : %s \n", filename, strerror(errno));
    exit(2);
}

static void* malloc_X(size_t size)
{
    void* const buff = malloc(size);
    if (buff) return buff;
    /* error */
    printf("malloc: %s \n", strerror(errno));
    exit(3);
}
#endif

bool zstd_load(std::vector<char>* buf, const char* filename)
{
#ifdef ENABLE_ZSTD
    off_t const buffSize = fsize_X(filename);
    FILE* const inFile = fopen_X(filename, "rb");
    void* const buffer = malloc_X(buffSize);
    size_t const readSize = fread(buffer, 1, buffSize, inFile);
    if (readSize != (size_t)buffSize) {
        printf("fread: %s : %s \n", filename, strerror(errno));
        exit(4);
    }
    fclose(inFile);

    unsigned long long const rSize = ZSTD_getDecompressedSize(buffer, buffSize);
    if (rSize==0) {
        printf("%s : original size unknown \n", filename);
        exit(5);
    }

    buf->resize(rSize);

    size_t const dSize = ZSTD_decompress(buf->data(), rSize, buffer, buffSize);

    if (dSize != rSize) {
        printf("error decoding %s : %s \n", filename, ZSTD_getErrorName(dSize));
        exit(7);
    }

    free(buffer);

    return true;
#else
  return false;
#endif
}

const char* get_file_data(size_t *len, const char* filename)
{

  const char *ext = strrchr(filename, '.');

  size_t data_len = 0;
  const char* data = nullptr;

  if (strcmp(ext, ".gz") == 0) {
    // gzipped data.

    std::vector<char> buf;
    bool ret = gz_load(&buf, filename);

    if (ret) {
      char *p = static_cast<char*>(malloc(buf.size() + 1));  // @fixme { implement deleter }
      memcpy(p, &buf.at(0), buf.size());
      p[buf.size()] = '\0';
      data = p;
      data_len = buf.size();
    }

  } else if (strcmp(ext, ".zst") == 0) {
    printf("zstd\n");
    // Zstandard data.

    std::vector<char> buf;
    bool ret = zstd_load(&buf, filename);

    if (ret) {
      char *p = static_cast<char*>(malloc(buf.size() + 1));  // @fixme { implement deleter }
      memcpy(p, &buf.at(0), buf.size());
      p[buf.size()] = '\0';
      data = p;
      data_len = buf.size();
    }
  } else {

    data = mmap_file(&data_len, filename);

  }

  (*len) = data_len;
  return data;
}


bool LoadObjAndConvert(float bmin[3], float bmax[3], const char* filename, int num_threads, bool verbose)
{
  tinyobj_opt::attrib_t attrib;
  std::vector<tinyobj_opt::shape_t> shapes;
  std::vector<tinyobj_opt::material_t> materials;

  auto load_t_begin = std::chrono::high_resolution_clock::now();
  size_t data_len = 0;
  const char* data = get_file_data(&data_len, filename);
  if (data == nullptr) {
    printf("failed to load file\n");
    exit(-1);
    return false;
  }
  auto load_t_end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> load_ms = load_t_end - load_t_begin;
  if (verbose) {
    std::cout << "filesize: " << data_len << std::endl;
    std::cout << "load time: " << load_ms.count() << " [msecs]" << std::endl;
  }


  tinyobj_opt::LoadOption option;
  option.req_num_threads = num_threads;
  option.verbose = verbose;
  bool ret = parseObj(&attrib, &shapes, &materials, data, data_len, option);

  if (!ret) {
	  std::cerr << "Failed to parse .obj" << std::endl;
	  return false;
  }
  bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
  bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();

  //std::cout << "vertices.size() = " << attrib.vertices.size() << std::endl;
  //std::cout << "normals.size() = " << attrib.normals.size() << std::endl;

  {
        DrawObject o;
        std::vector<float> vb; // pos(3float), normal(3float), color(3float)
        size_t face_offset = 0;
        for (size_t v = 0; v < attrib.face_num_verts.size(); v++) {
          assert(attrib.face_num_verts[v] % 3 == 0); // assume all triangle face(multiple of 3).
          for (size_t f = 0; f < attrib.face_num_verts[v] / 3; f++) {
            tinyobj_opt::index_t idx0 = attrib.indices[face_offset+3*f+0];
            tinyobj_opt::index_t idx1 = attrib.indices[face_offset+3*f+1];
            tinyobj_opt::index_t idx2 = attrib.indices[face_offset+3*f+2];

            float v[3][3];
            for (int k = 0; k < 3; k++) {
              int f0 = idx0.vertex_index;
              int f1 = idx1.vertex_index;
              int f2 = idx2.vertex_index;
              assert(f0 >= 0);
              assert(f1 >= 0);
              assert(f2 >= 0);

              v[0][k] = attrib.vertices[3*f0+k];
              v[1][k] = attrib.vertices[3*f1+k];
              v[2][k] = attrib.vertices[3*f2+k];
              bmin[k] = std::min(v[0][k], bmin[k]);
              bmin[k] = std::min(v[1][k], bmin[k]);
              bmin[k] = std::min(v[2][k], bmin[k]);
              bmax[k] = std::max(v[0][k], bmax[k]);
              bmax[k] = std::max(v[1][k], bmax[k]);
              bmax[k] = std::max(v[2][k], bmax[k]);
            }

            float n[3][3];

            if (attrib.normals.size() > 0) {
              int nf0 = idx0.normal_index;
              int nf1 = idx1.normal_index;
              int nf2 = idx2.normal_index;

              if (nf0 >= 0 && nf1 >= 0 && nf2 >= 0) {
                assert(3*nf0+2 < attrib.normals.size());
                assert(3*nf1+2 < attrib.normals.size());
                assert(3*nf2+2 < attrib.normals.size());
                for (int k = 0; k < 3; k++) {
                  n[0][k] = attrib.normals[3*nf0+k];
                  n[1][k] = attrib.normals[3*nf1+k];
                  n[2][k] = attrib.normals[3*nf2+k];
                }
              } else {
                // compute geometric normal
                CalcNormal(n[0], v[0], v[1], v[2]);
                n[1][0] = n[0][0]; n[1][1] = n[0][1]; n[1][2] = n[0][2];
                n[2][0] = n[0][0]; n[2][1] = n[0][1]; n[2][2] = n[0][2];
              }
            } else {
              // compute geometric normal
              CalcNormal(n[0], v[0], v[1], v[2]);
              n[1][0] = n[0][0]; n[1][1] = n[0][1]; n[1][2] = n[0][2];
              n[2][0] = n[0][0]; n[2][1] = n[0][1]; n[2][2] = n[0][2];
            }

            for (int k = 0; k < 3; k++) {
              vb.push_back(v[k][0]);
              vb.push_back(v[k][1]);
              vb.push_back(v[k][2]);
              vb.push_back(n[k][0]);
              vb.push_back(n[k][1]);
              vb.push_back(n[k][2]);
              // Use normal as color.
              float c[3] = {n[k][0], n[k][1], n[k][2]};
              float len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
              if (len2 > 1.0e-6f) {
                float len = sqrtf(len2);

                c[0] /= len;
                c[1] /= len;
                c[2] /= len;
              }
              vb.push_back(c[0] * 0.5 + 0.5);
              vb.push_back(c[1] * 0.5 + 0.5);
              vb.push_back(c[2] * 0.5 + 0.5);
            }
          }
          face_offset += attrib.face_num_verts[v];
        }

        o.vb = 0;
        o.numTriangles = 0;
        if (vb.size() > 0) {
          glGenBuffers(1, &o.vb);
          glBindBuffer(GL_ARRAY_BUFFER, o.vb);
          glBufferData(GL_ARRAY_BUFFER, vb.size() * sizeof(float), &vb.at(0), GL_STATIC_DRAW);
          o.numTriangles = vb.size() / 9 / 3;
        }

        gDrawObjects.push_back(o);
  }

  printf("bmin = %f, %f, %f\n", bmin[0], bmin[1], bmin[2]);
  printf("bmax = %f, %f, %f\n", bmax[0], bmax[1], bmax[2]);

  return true;
}

void reshapeFunc(GLFWwindow* window, int w, int h)
{
  (void)window;
  // for retinal display.
  int fb_w, fb_h;
  glfwGetFramebufferSize(window, &fb_w, &fb_h);

  glViewport(0, 0, fb_w, fb_h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (float)w / (float)h, 0.01f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  width = w;
  height = h;
}

void keyboardFunc(GLFWwindow *window, int key, int scancode, int action, int mods) {
  (void)window;
  (void)scancode;
  (void)mods;
    if(action == GLFW_PRESS || action == GLFW_REPEAT){
        // Move camera
        float mv_x = 0, mv_y = 0, mv_z = 0;
        if(key == GLFW_KEY_K) mv_x += 1;
        else if(key == GLFW_KEY_J) mv_x += -1;
        else if(key == GLFW_KEY_L) mv_y += 1;
        else if(key == GLFW_KEY_H) mv_y += -1;
        else if(key == GLFW_KEY_P) mv_z += 1;
        else if(key == GLFW_KEY_N) mv_z += -1;
        //camera.move(mv_x * 0.05, mv_y * 0.05, mv_z * 0.05);
        // Close window
        if(key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GL_TRUE);

        //init_frame = true;
    }
}

void clickFunc(GLFWwindow* window, int button, int action, int mods){
  (void)window;
  (void)mods;
    if(button == GLFW_MOUSE_BUTTON_LEFT){
        if(action == GLFW_PRESS){
            mouseLeftPressed = true;
            trackball(prev_quat, 0.0, 0.0, 0.0, 0.0);
        } else if(action == GLFW_RELEASE){
            mouseLeftPressed = false;
        }
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT){
        if(action == GLFW_PRESS){
            mouseRightPressed = true;
        } else if(action == GLFW_RELEASE){
            mouseRightPressed = false;
        }
    }
    if(button == GLFW_MOUSE_BUTTON_MIDDLE){
        if(action == GLFW_PRESS){
            mouseMiddlePressed = true;
        } else if(action == GLFW_RELEASE){
            mouseMiddlePressed = false;
        }
    }
}

void motionFunc(GLFWwindow* window, double mouse_x, double mouse_y){
  (void)window;
  float rotScale = 1.0f;
  float transScale = 2.0f;

    if(mouseLeftPressed){
      trackball(prev_quat,
          rotScale * (2.0f * prevMouseX - width) / (float)width,
          rotScale * (height - 2.0f * prevMouseY) / (float)height,
          rotScale * (2.0f * mouse_x - width) / (float)width,
          rotScale * (height - 2.0f * mouse_y) / (float)height);

      add_quats(prev_quat, curr_quat, curr_quat);
    } else if (mouseMiddlePressed) {
      eye[0] -= transScale * (mouse_x - prevMouseX) / (float)width;
      lookat[0] -= transScale * (mouse_x - prevMouseX) / (float)width;
      eye[1] += transScale * (mouse_y - prevMouseY) / (float)height;
      lookat[1] += transScale * (mouse_y - prevMouseY) / (float)height;
    } else if (mouseRightPressed) {
      eye[2] += transScale * (mouse_y - prevMouseY) / (float)height;
      lookat[2] += transScale * (mouse_y - prevMouseY) / (float)height;
    }

    // Update mouse point
    prevMouseX = mouse_x;
    prevMouseY = mouse_y;
}

void Draw(const std::vector<DrawObject>& drawObjects)
{
  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_FILL);

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glColor3f(1.0f, 1.0f, 1.0f);
  for (size_t i = 0; i < drawObjects.size(); i++) {
    DrawObject o = drawObjects[i];
    if (o.vb < 1) {
      continue;
    }

    glBindBuffer(GL_ARRAY_BUFFER, o.vb);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 36, (const void*)0);
    glNormalPointer(GL_FLOAT, 36, (const void*)(sizeof(float)*3));
    glColorPointer(3, GL_FLOAT, 36, (const void*)(sizeof(float)*6));

    glDrawArrays(GL_TRIANGLES, 0, 3 * o.numTriangles);
    CheckErrors("drawarrays");
  }

  // draw wireframe
  glDisable(GL_POLYGON_OFFSET_FILL);
  glPolygonMode(GL_FRONT, GL_LINE);
  glPolygonMode(GL_BACK, GL_LINE);

  glColor3f(0.0f, 0.0f, 0.4f);
  for (size_t i = 0; i < drawObjects.size(); i++) {
    DrawObject o = drawObjects[i];
    if (o.vb < 1) {
      continue;
    }

    glBindBuffer(GL_ARRAY_BUFFER, o.vb);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 36, (const void*)0);
    glNormalPointer(GL_FLOAT, 36, (const void*)(sizeof(float)*3));

    glDrawArrays(GL_TRIANGLES, 0, 3 * o.numTriangles);
    CheckErrors("drawarrays");
  }
}

static void Init() {
  trackball(curr_quat, 0, 0, 0, 0);

  eye[0] = 0.0f;
  eye[1] = 0.0f;
  eye[2] = 3.0f;

  lookat[0] = 0.0f;
  lookat[1] = 0.0f;
  lookat[2] = 0.0f;

  up[0] = 0.0f;
  up[1] = 1.0f;
  up[2] = 0.0f;
}


int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cout << "view input.obj <num_threads> <benchark_only> <verbose>" << std::endl;
    return 0;
  }

  bool benchmark_only = false;
  int num_threads = -1;
  bool verbose = false;

  if (argc > 2) {
    num_threads = atoi(argv[2]);
  }

  if (argc > 3) {
    benchmark_only = (atoi(argv[3]) > 0) ? true : false;
  }

  if (argc > 4) {
    verbose = true;
  }

  if (benchmark_only) {

    tinyobj_opt::attrib_t attrib;
    std::vector<tinyobj_opt::shape_t> shapes;
    std::vector<tinyobj_opt::material_t> materials;

    size_t data_len = 0;
    const char* data = get_file_data(&data_len, argv[1]);
    if (data == nullptr) {
      printf("failed to load file\n");
      exit(-1);
      return false;
    }

    if (data_len < 4) {
      printf("Empty file\n");
      exit(-1);
      return false;
    }
    printf("filesize: %d\n", (int)data_len);
    tinyobj_opt::LoadOption option;
    option.req_num_threads = num_threads;
    option.verbose = true;

    bool ret = parseObj(&attrib, &shapes, &materials, data, data_len, option);

    return ret;
  }

  Init();

  std::cout << "Initialize GLFW..." << std::endl;

  if(!glfwInit()){
    std::cerr << "Failed to initialize GLFW." << std::endl;
    return -1;
  }

  std::cout << "GLFW Init OK." << std::endl;


  window = glfwCreateWindow(width, height, "Obj viewer", NULL, NULL);
  if(window == NULL){
    std::cerr << "Failed to open GLFW window. " << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Callback
  glfwSetWindowSizeCallback(window, reshapeFunc);
  glfwSetKeyCallback(window, keyboardFunc);
  glfwSetMouseButtonCallback(window, clickFunc);
  glfwSetCursorPosCallback(window, motionFunc);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW." << std::endl;
    return -1;
  }

  reshapeFunc(window, width, height);

  float bmin[3], bmax[3];
  if (false == LoadObjAndConvert(bmin, bmax, argv[1], num_threads, verbose)) {
    printf("failed to load & conv\n");
    return -1;
  }

  float maxExtent = 0.5f * (bmax[0] - bmin[0]);
  if (maxExtent < 0.5f * (bmax[1] - bmin[1])) {
    maxExtent = 0.5f * (bmax[1] - bmin[1]);
  }
  if (maxExtent < 0.5f * (bmax[2] - bmin[2])) {
    maxExtent = 0.5f * (bmax[2] - bmin[2]);
  }

  while(glfwWindowShouldClose(window) == GL_FALSE) {
    glfwPollEvents();
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // camera & rotate
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    GLfloat mat[4][4];
    gluLookAt(eye[0], eye[1], eye[2], lookat[0], lookat[1], lookat[2], up[0], up[1], up[2]);
    build_rotmatrix(mat, curr_quat);
    glMultMatrixf(&mat[0][0]);

    // Fit to -1, 1
    glScalef(1.0f / maxExtent, 1.0f / maxExtent, 1.0f / maxExtent);

    // Centerize object.
    glTranslatef(-0.5*(bmax[0] + bmin[0]), -0.5*(bmax[1] + bmin[1]), -0.5*(bmax[2] + bmin[2]));

    Draw(gDrawObjects);

    glfwSwapBuffers(window);
  }

  glfwTerminate();
}
