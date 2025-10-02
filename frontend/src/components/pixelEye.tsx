import React, { useEffect, useState } from "react";

interface PixelEyeProps {
  winking?: boolean;
  size?: number; // pixel size
  rolledUp?: boolean; // 👈 new prop
}

const PixelEye: React.FC<PixelEyeProps> = ({
  winking = false,
  size = 12,
  rolledUp = false,
}) => {
  const [open, setOpen] = useState(true);

  useEffect(() => {
    if (rolledUp) return; // 👈 when eyes are rolled up, disable blinking/winking

    let blinkTimeout: ReturnType<typeof setTimeout>;

    const blink = () => {
      setOpen(false);
      blinkTimeout = setTimeout(() => setOpen(true), 300); // wink duration
    };

    const interval = setInterval(() => {
      if (!winking) blink();
      else if (Math.random() > 0.5) blink();
    }, 3000);

    return () => {
      clearInterval(interval);
      clearTimeout(blinkTimeout);
    };
  }, [winking, rolledUp]);

  const pixelStyle = (filled: boolean) => ({
    width: `${size}px`,
    height: `${size}px`,
    backgroundColor: filled ? "#3b82f6" : "transparent",
    transition: "background-color 0.2s",
  });

  // Eye layouts (5x5)
  const openLayout = [
    [0, 1, 1, 1, 0],
    [1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1],
    [0, 1, 1, 1, 0],
  ];

  /*
  const winkLayout = [
    [1, 0, 0, 0, 0],
    [0, 1, 0, 0, 0],
    [0, 0, 1, 0, 0],
    [0, 1, 0, 0, 0],
    [1, 0, 0, 0, 0],
  ];*/

  const winkLayout = [
    [0, 0, 1, 0, 0],
    [0, 1, 0, 1, 0],
    [1, 0, 0, 0, 1],
    [0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0],
  ];

  // 👀 rolled up (eyes looking upward)
  const rolledUpLayout = [
    [1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1],
    [0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0],
  ];

  let layout = open ? openLayout : winkLayout;
  if (rolledUp) layout = rolledUpLayout;

  return (
    <div className="flex gap-1">
      <div className="flex flex-col gap-1">
        {layout.map((row, rowIndex) => (
          <div key={rowIndex} className="flex gap-1">
            {row.map((filled, colIndex) => (
              <div key={colIndex} style={pixelStyle(Boolean(filled))} />
            ))}
          </div>
        ))}
      </div>
    </div>
  );
};

export default PixelEye;
