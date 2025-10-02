import React from "react";

const InputSkeleton: React.FC<{ width?: string }> = ({ width = "w-full" }) => {
  return (
    <div className={`h-10 bg-gray-300 rounded ${width} animate-pulse`}></div>
  );
};

export default InputSkeleton;
