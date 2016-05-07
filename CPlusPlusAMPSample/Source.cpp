#include<stdlib.h>
#include <amp.h>
#include <iostream>

using namespace concurrency;
using namespace std;

void ComputePrintArrayView(array_view<int>&);
vector<int> ComputePrintPrallelArray(concurrency::array<int>&);

// Win32 console application project

int main()
{
	const int arraySize = 11;
	// Hello world, each -1
	int v[arraySize] = { 'G', 'd', 'k', 'k', 'n', 31, 'v', 'n', 'q', 'k', 'c' };

	// N dimensional view of a data held in another container array_view<T,N>
	array_view<int> av(arraySize, v);

	// Hello parallel world !
	parallel_for_each(av.extent, [=](index<1> idx) restrict(amp)
	{
		av[idx] += 1;
	});

	for (unsigned int i = 0; i < 11; i++)
		cout << static_cast<char>(av[i]);
	cout << endl;


	// Conventional array
	int oneDimArray[] = { 4,2,6,8,9 };

	// The prepective of a multidimensional array
	array_view<const int, 1> oneDimArrayView(5, oneDimArray);

	// The index in multi-dimensional space, here it's only 1D
	index<1> idxOneDim(2);
	// Accessing an array item using an index
	cout << oneDimArrayView(idxOneDim) << endl; // 6
	cout << oneDimArrayView[idxOneDim] << endl; // 6

	cout << oneDimArrayView(2) << endl; // 6
	cout << oneDimArrayView[2] << endl; // 6
	//cout << oneDimArrayView(2, 3) << endl;	// Compile time error, wow!


	// Let's create a 2d array
	cout << "2d array" << endl;

	int twoDimArray[] = { 4,5,6
		,7,8,9 };

	array_view<const int, 2> twoDimArrayView(2, 3, twoDimArray);
	cout << twoDimArrayView(0, 1) << endl; // 5
	cout << twoDimArrayView[1][2] << endl; // 9
	//cout << twoDimArrayView[1,2] << endl;	// Compile time error


	// 3d arrays
	cout << "3d array" << endl;

	int threeDimArray[] = {
		// Depth = 0, rows and columns
		1, 2, 3, 4,
		5, 6, 7, 8,
		9, 10, 11, 12,

		// Depth = 1, rows and columns
		21, 22, 23, 24,
		25, 26, 27, 28,
		29, 210, 211, 212 };

	// depth = 2, the rows = 3, and the columns = 4
	array_view<int, 3> threeDimArrayView(2, 3, 4, threeDimArray);

	// Specifies the element at 3, 1, 0.
	index<3> ind(0, 1, 3);
	cout << threeDimArrayView[ind] << endl; // 8
	cout << threeDimArrayView[0][1][3] << endl; // 8
	cout << threeDimArrayView(0, 1, 13) << endl; // it counted 13 indexes from d = 0, row = 1, playing this way isn't good though


	/*
	The extent Class (C++ AMP) specifies the length of the data in each dimension of the array or array_view object.
	You can create an extent and use it to create an array or array_view object.
	You can also retrieve the extent of an existing array or array_view object.
	The following example prints the length of the extent in each dimension of an array_view object.
	*/
	cout << "Extent info" << endl;
	cout << threeDimArrayView.extent[0] << endl; // Depth
	cout << threeDimArrayView.extent[1] << endl; // Rows
	cout << threeDimArrayView.extent[2] << endl; // Columns
												 //cout << threeDimArrayView.extent[3] << endl;	// ?? - garbage


	// Copying and processing

	/*
	When an array object is constructed, a deep copy of the data is created on the accelerator if you use
	a constructor that includes a pointer to the data set. The kernel function modifies the copy on the accelerator.
	When the execution of the kernel function is finished, you must copy the data back to the source data structure.
	The following example multiplies each element in a vector by 10.
	After the kernel function is finished,the vector conversion operator is used to copy the data back into the vector object.
	*/

	vector<int> vecData(5);

	// Initialize the vector
	for (int count = 0; count < 5; count++)
		vecData[count] = count;


	/*
	array views can't be passed to kernel functions by reference, hence we must
	use an concurrency::array objects and them copy them back to memory
	*/

	cout << "Printing the parallel array" << endl;

	// When using the concurrency::array the begin and start point must be provided
	// the concurrency::array creates a deep copy on the initial object
	concurrency::array<int, 1> parallelArray(5, vecData.begin(), vecData.end());
	vector<int>vecDataComputed = ComputePrintPrallelArray(parallelArray);

	for (unsigned int i = 0; i < vecData.size(); i++)
		cout << vecDataComputed[i] << " - " << i << endl;

	// The concurrency::array_view creates a shallow copy on the initial object which 
	// leads to the modification of the base object
	cout << "Printing the array view" << endl;
	array_view<int, 1> parallelArrayView(5, vecData);
	ComputePrintArrayView(parallelArrayView);

	system("pause");
}

// C++ AMP Reference https://msdn.microsoft.com/en-us/library/hh265137.aspx
// C++ AMP Overview https://msdn.microsoft.com/en-us/library/hh265136.aspx
// Hello world example https://blogs.msdn.microsoft.com/nativeconcurrency/2012/03/04/hello-world-in-c-amp/

void ComputePrintArrayView(array_view<int>& parallelArrayView)
{
	parallel_for_each(
		parallelArrayView.extent,
		[=](index<1> idx) restrict(amp)
	{
		parallelArrayView[idx] *= parallelArrayView[idx];
	});

	for (int i = 0; i < parallelArrayView.extent[0]; i++)
		cout << parallelArrayView[i] << " - " << i << endl;
}

vector<int> ComputePrintPrallelArray(concurrency::array<int>& parallelArray)
{
	parallel_for_each(
		parallelArray.extent,
		[=, &parallelArray](index<1> idx) restrict(amp)
	{
		parallelArray[idx] *= parallelArray[idx];
	});
	vector<int> retVal = parallelArray; // The array computed on parallel processors must be copied back
	return retVal;
}
