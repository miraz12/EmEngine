using System.Numerics;
namespace Entities
{
    public class Player
    {
        public Vector3 position = new Vector3(0, 0, 0);
        public Vector3 forward = new Vector3(0, 0, 0);
        public Vector3 right = new Vector3(0, 0, 0);
        public Vector3 acceleration = new Vector3(0, 0, 0);

        public void update(float dt)
        {
            // TODO: Do position modification stuff
        }
    }
}
