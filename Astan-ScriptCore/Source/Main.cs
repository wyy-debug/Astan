using System;

namespace Astan { 
    public class Main
    {
        public Main()
        {
            Console.WriteLine("Main constructor!");
        }

        public void PrintMessage()
        {
            Console.WriteLine("Hello World from C#");
        }

        public void PrintInt(int value)
        {
            Console.WriteLine($"C# say: {value}");
        }
        
        public void PrintInts(int value,int value2)
        {
            Console.WriteLine($"C# say: {value} and {value2}");
        }

        public void PrintCustomMessage(string message)
        {
            Console.WriteLine($"C# say: {message}");
        }
    }
}
