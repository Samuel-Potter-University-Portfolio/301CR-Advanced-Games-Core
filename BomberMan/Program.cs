using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using PotterEngine;

namespace BomberMan
{
    class Program
    {
        static void Main(string[] args)
        {
            EngineInfo info = new EngineInfo(args);
            Engine engine = new Engine(info);

            Game game = new Game("Test Game");
            engine.Launch(game);
            engine.Close();
            
        }
    }

    
}
