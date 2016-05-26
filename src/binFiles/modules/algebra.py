# Geometric Figures  Copyright (C) 2015--2016  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module contains several functions of linear algebra

module_help="""
Module algebra contains several functions from linear algebra,
it has only Python interface:

  dotProduct(vector1, vector2)                 -dot product of vectors
  vectLen(vector)                              -euclidian vector length
  vectDiff(vector1, vector2)                   -difference of vectors
  vectSum(vector1, ...)                        -sum of vectors
  vectMult(multiplier, vector)                 -multiple of vector
  vectAvg(vector1, ...)                        -arithmetic mean of the vectors
  pointsDist(point1, point2)                   -euclidian distance of the points
  orthogonalizeVect(vector, orthonormalBasis)  -orthogolalization of vector
  orthonormalizeBasis(basis)                   -Gram-Schmidt orthonormalization
  orthonormalBasisFromPoints(points)           -gets orthonormal basis from points
  hyperplaneFromPoints(points, positivePoint)  -creates Hyperplane object from points

  Hyperplane(normal, normalPos)  -hyperplane object
    .normal                      -normal vector
    .normalPos                   -position within the normal vector
    .orientedDistance(point)     -distance in normalized normal
    .inverse()                   -creates Hyperplane returning inverse oriented distance

uses modules: [helpMod]

For more information see algebra.py
"""

try:
	import helpMod
	helpMod.addModule("algebra", module_help)
except ImportError:
	pass

def dotProduct(vector1, vector2):
	return sum(map(lambda z: z[0]*z[1], zip(vector1, vector2)))

def vectLen(vector):
	return sum(map(lambda x: x**2, vector))**0.5

def vectDiff(vector1, vector2):
	return tuple(map(lambda z:z[0]-z[1], zip(vector1, vector2)))

def vectSum(*vectors):
	return tuple(map(lambda z:sum(z), zip(*vectors)))

def vectMult(mult, vector):
	return tuple(map(lambda x: mult*x, vector))

def vectAvg(*vectors):
	return vectMult(1.0/len(vectors), vectSum(*vectors))

def pointsDist(point1, point2):
	return vectLen(vectDiff(point1, point2))

def orthogonalizeVect(vector, orthonormalBasis):
	for basisVect in orthonormalBasis:
		lenInBasisVect=dotProduct(basisVect, vector)
		vector=vectSum(vector, vectMult(-lenInBasisVect, basisVect))
	return vector

def orthonormalizeBasis(basis):
	ortBasis=[]
	for vector in basis:
		vector=orthogonalizeVect(vector, ortBasis)
		vectorLen=vectLen(vector)
		if vectorLen>0.00001:
			vector=vectMult(1/vectorLen, vector)
			ortBasis.append(vector)
	return ortBasis

def orthonormalBasisFromPoints(points):
	points=list(points)
	point=points.pop()
	return orthonormalizeBasis(vectDiff(p,point) for p in points)

import random

# positivePoint should have positive oriented distance from the hyperplane
def hyperplaneFromPoints(points, positivePoint=None):
	if not positivePoint:
		positivePoint=(0,)*len(points[0])
	ortBasis=orthonormalBasisFromPoints(points)
	normal=orthogonalizeVect(points[0], ortBasis)
	normalLen=normalPos=vectLen(normal)
	while normalLen<0.01:
		normal=orthogonalizeVect(list(random.random()*2-1 for c in normal), ortBasis)
		normalLen=vectLen(normal)

	hyperplane=Hyperplane(normal, normalPos/normalLen)
	if hyperplane.orientedDistance(positivePoint)<0:
		hyperplane=hyperplane.inverse()
	return hyperplane

class Hyperplane:

	# Creates Hyperplane from its normal and position within it
	# (normalPos * normal = point in the Hyperplane)
	def __init__(self, normal, normalPos=0):
		l=vectLen(normal)
		self.normal=map(lambda x: x/l, normal)
		self.normalPos=normalPos*l

	# Distance in the direction of the normal
	def orientedDistance(self, coords):
		return dotProduct(self.normal, coords)-self.normalPos

	def inverse(self):
		return Hyperplane(vectMult(-1, self.normal), -self.normalPos)


