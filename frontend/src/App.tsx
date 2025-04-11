import { ThemeProvider } from "@/components/providers/ThemeProvider";
import { ThemeToggle } from "./components/ThemeToggle";
import { Card, CardContent, CardHeader } from "./components/ui/card";
import { useEffect, useState } from "react";

type ESPData = {
  voltage: number;
  current: number;
  power: number;
};

function App() {
  const [data, setData] = useState<ESPData>({
    voltage: 0,
    current: 0,
    power: 0,
  });

  useEffect(() => {
    const fetchData = async () => {
      try {
        const res = await fetch("/api/v1/data");
        const json = await res.json();
        setData(json);
      } catch (err) {
        console.error("Error fetching ESP data:", err);
      }
    };

    fetchData(); // initial fetch
    const interval = setInterval(fetchData, 2000); // refresh every 2s
    return () => clearInterval(interval);
  }, []);

  return (
    <ThemeProvider defaultTheme="dark" storageKey="vite-ui-theme">
      <div className="p-10 flex flex-col space-y-5 h-screen w-screen items-center">
        <div className="flex items-center justify-between w-full">
          <h1 className="text-4xl font-bold">VanMon v1.0</h1>
          <ThemeToggle />
        </div>
        <div className="w-full grid grid-cols-1 gap-4 ">
          <Card className="col-span-1 bg-blue-950">
            <CardContent className="flex flex-col space-y-4">
              <div>
                <div>Total Battery Voltage</div>
                <div className="text-2xl font-bold">{data.voltage}</div>
              </div>
              <div>
                <div>Appliances Connected</div>
                <div className="text-2xl font-bold">4</div>
              </div>
              <div>
                <div>Total Load</div>
                <div className="text-2xl font-bold">832.34 mW</div>
              </div>
              <div>
                <div>Time Until drained</div>
                <div className="text-2xl font-bold">4 Hours</div>
              </div>
            </CardContent>
          </Card>
          <Card className="col-span-1">
            <CardHeader>
              <h2 className="text-xl font-bold">Lights</h2>
            </CardHeader>
            <CardContent className="flex flex-col space-y-4">
              <div>
                <div>Current Draw</div>
                <div className="text-2xl font-bold">{data.current} mA</div>
              </div>
              <div>
                <div>Power Draw</div>
                <div className="text-2xl font-bold">{data.power} mW</div>
              </div>
            </CardContent>
          </Card>
          <Card className="col-span-1">
            <CardHeader>
              <h2 className="text-xl font-bold">Fridge</h2>
            </CardHeader>
            <CardContent className="flex flex-col space-y-4">
              <div>
                <div>Current Draw</div>
                <div className="text-2xl font-bold">92.5 mA</div>
              </div>
              <div>
                <div>Power Draw</div>
                <div className="text-2xl font-bold">432.32 mW</div>
              </div>
            </CardContent>
          </Card>
          <Card className="col-span-1">
            <CardHeader>
              <h2 className="text-xl font-bold">Charging Ports</h2>
            </CardHeader>
            <CardContent className="flex flex-col space-y-4">
              <div>
                <div>Current Draw</div>
                <div className="text-2xl font-bold">92.5 mA</div>
              </div>
              <div>
                <div>Power Draw</div>
                <div className="text-2xl font-bold">432.32 mW</div>
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    </ThemeProvider>
  );
}

export default App;
