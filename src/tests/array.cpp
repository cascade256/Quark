#include <cstring>


void testArrays() {
	{
		Array<int> nums;
		arrayInit(&nums);
		arrayAdd(&nums, 1);

		assert(nums.data[0] == 1 && nums.len == 1 && nums.capacity >= 1);
	}
	{
		Array<int> nums;
		arrayInit(&nums);

		int a[] = { 1, 2, 3, 5, 6 };

		arrayAdd(&nums, a, 5);
		arrayInsert(&nums, 3, 4);
		arrayInsert(&nums, 6, 7);

		for (int i = 0; i < 7; i++) {
			assert(nums[i] == i + 1);
		}
		assert(nums.len == 7 && nums.capacity >= 7);
	}
	//Test arrayRemoveRange()
	{
		Array<int> nums;
		arrayInit(&nums);

		int a[] = { 1, 2, 3, 3, 3, 4, 5, 6 };

		arrayAdd(&nums, a, 8);

		arrayRemoveRange(&nums, 3, 5);

		for (int i = 0; i < 6; i++) {
			assert(nums[i] == i + 1);
		}
		assert(nums.len == 6 && nums.capacity >= 6);
	}
}