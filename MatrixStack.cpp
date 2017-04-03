#include "MatrixStack.h"
#include "GL\freeglut.h";
using namespace std;

MatrixStack::MatrixStack()
{
	// Initialize the matrix stack with the identity matrix.
	Matrix4f identity_mat = Matrix4f().identity();
	m_matrices.push_back(identity_mat);

}

// an iterator is of a poiinter form.
void MatrixStack::clear()
{
	// Revert to just containing the identity matrix.
	m_matrices.clear();
	m_matrices.push_back(Matrix4f::identity());
	//m_matrices.erase(m_matrices.begin() + 1, m_matrices.end());
}

Matrix4f MatrixStack::top()
{
	// Return the top of the stack

	return m_matrices.back();
}

//const means that the pointer or reference (in this case m which is a reference
//cannot be used for a write or read-modify-write operation.
void MatrixStack::push( const Matrix4f& m )
{
	// Push m onto the stack.
	// Your stack should have OpenGL semantics:
	// the new top should be the old top multiplied by m
	// o.T = w.T O

	m_matrices.push_back(top() * m);
	glLoadMatrixf(top());
	//i guess this means that coordinates
	
}

void MatrixStack::pop()
{
	// Remove the top element from the stack
	m_matrices.pop_back();
	glLoadMatrixf(top());
}
