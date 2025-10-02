import React from "react";

const PageSkeleton: React.FC = () => {
  return (
    <div className="min-h-screen w-full flex flex-col items-center justify-center px-4 bg-gray-100 dark:bg-gray-900 text-gray-900 dark:text-gray-100 transition-colors">
      <div className="animate-pulse p-4 space-y-4">
        <div className="h-8 bg-gray-300 rounded w-1/4"></div>
        <div className="h-6 bg-gray-300 rounded w-3/4"></div>
        <div className="h-6 bg-gray-300 rounded w-2/4"></div>
        <div className="h-10 bg-gray-300 rounded w-full"></div>
      </div>
    </div>
  );
};

export default PageSkeleton;
