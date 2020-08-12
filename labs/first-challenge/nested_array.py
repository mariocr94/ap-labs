def getLength(a):
	if len(a) == 0:
		return 0

	c=0
	for i in a:
		if isinstance(i, list):
			c+=getLength(i)
		else:
			c+=1
	return c

print(getLength([1, [2, 3]]))
print(getLength([1, [2, [3, 4]]]))
print(getLength([1, [2, [3, [4, [5, 6]]]]]))
print(getLength([1, [2], 1, [2], 1]))
print(getLength([]))

