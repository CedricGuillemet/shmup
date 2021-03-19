#include <map>
#include <camera.h>
#include <vector>
#include <algorithm>
struct Animation
{
    Animation()
    {
        mKeys.push_back({0, {{0,0,0}, {0,0,0}}});
        mKeys.push_back({ 99, {{0,1,0}, {0,0,0}} });
    }
    int mStartFrame{0}, mEndFrame{100};

    struct Key
    {
        int mFrame;
        Camera mCamera;
    };
    std::vector<Key> mKeys;

    void MakeKey(int frame, Camera camera)
    {
        for (int i = 0; i < mKeys.size(); i++)
        {
            auto& key = mKeys[i];
            if (key.mFrame == frame)
            {
                key.mCamera = camera;
                return;
            }
        }
        mKeys.push_back({frame, camera});
        SortKeys();
    }

    Camera Evaluate(int frame)
    {
        for (int i = 0;i< mKeys.size();i++)
        {
            const auto& key = mKeys[i];
            if (key.mFrame == frame)
            {
                return key.mCamera;
            }
            else if (key.mFrame > frame)
            {
                if (i > 0)
                {
                    const auto& prevKey = mKeys[i-1];
                    float t = float(frame - prevKey.mFrame) / float(key.mFrame - prevKey.mFrame);

                    Camera res;
                    res.mPosition.Lerp(prevKey.mCamera.mPosition, key.mCamera.mPosition, t);
                    res.mAngles.Lerp(prevKey.mCamera.mAngles, key.mCamera.mAngles, t);
                    return res;
                }
            }
        }
        return mKeys.back().mCamera;
    }

    void SortKeys()
    {
        std::sort(mKeys.begin(), mKeys.end(), [](const Key& a, const Key& b) {
            return a.mFrame < b.mFrame;
            });
    }
};