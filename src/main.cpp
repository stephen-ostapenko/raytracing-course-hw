#include <fstream>

#include "scene.h"
#include "image.h"

int main(int argc, char **argv) {
	std::ifstream in(argv[1]);
	std::ofstream out(argv[2]);

	Scene scene = read_scene(in);
	Image result = render_scene(scene);
	write_image(result, out);

	return 0;
}
