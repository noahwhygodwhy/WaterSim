#include "KDTree.hpp"

//TODO: multithread later





void processNode(const fvec3* balls, const size_t& numberOfPoints, const uint32_t* totalList, const uint32_t layer, const size_t receivedIndex)
{
	uint32_t axis = layer % 3;
	auto mySert = [balls, axis](size_t a, size_t b) {return balls[a][0] < balls[b][0]; };

	std::sort(totalList, totalList + numberOfPoints, mySert);

	uint32_t middleIdx = numberOfPoints/2 

}









KDTree makeKDTree(const fvec3* balls, const size_t& numberOfPoints) {
	size_t* totalList = new size_t[numberOfPoints];
	for (size_t i = 0; i < numberOfPoints; i++) {
		totalList[i] = i;
	}



	Axis axis = X;

	//auto mySort = [balls, axis](size_t a, size_t b) { return std::sort(a, b, [balls, axis](size_t x, size_t y) {return balls[x][axis] > balls[y][axis]; }); };


	auto mySert = [balls, axis](size_t a, size_t b) {return balls[a][0] < balls[b][0];};


	std::sort(totalList, totalList + numberOfPoints, mySert);

	for (int i = 0; i < numberOfPoints; i++) {
		printf("%i: %s\n", i, glm::to_string(balls[totalList[i]]).c_str());

	}
	exit(0);


	KDTree toReturn = KDTree();
	toReturn.size = numberOfPoints;


	//TODO: need to allocate the tree. How do i actually determine how much memory I need? Am I failing to recognize what's there
	//or is it actually complicated? https://www.geeksforgeeks.org/octree-insertion-and-searching/


	delete[] totalList;
}

