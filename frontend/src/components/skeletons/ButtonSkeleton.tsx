import React from "react";

const ButtonSkeleton: React.FC<{ width?: string }> = ({ width = "w-32" }) => {
  return (
    <div className={`h-10 bg-gray-300 rounded ${width} animate-pulse`}></div>
  );
};

export default ButtonSkeleton;
